/**
 * @file main.cpp
 *
 * This module holds the main() function, which is the entrypoint
 * to the program.
 *
 * © 2020 by Richard Walters
 */

#include "Leds.hpp"
#include "TimeKeeper.hpp"

#ifdef _WIN32
#include <crtdbg.h>
#endif /* _WIN32 */

#include <Http/Server.hpp>
#include <HttpNetworkTransport/HttpServerNetworkTransport.hpp>
#include <inttypes.h>
#include <SystemAbstractions/DiagnosticsSender.hpp>
#include <SystemAbstractions/DiagnosticsStreamReporter.hpp>
#include <SystemAbstractions/NetworkConnection.hpp>
#include <signal.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <StringExtensions/StringExtensions.hpp>
#include <thread>

namespace {

    /**
     * This function prints to the standard error stream information
     * about how to use this program.
     */
    void PrintUsageInformation() {
        fprintf(
            stderr,
            (
                "Usage: Lighter\n"
                "\n"
                "Run the HTTP server until SIGINT is signalled (Ctrl+C pressed).\n"
            )
        );
    }

    /**
     * This flag indicates whether or not the application should shut down.
     */
    bool shutDown = false;

    /**
     * This contains variables set through the operating system environment
     * or the command-line arguments.
     */
    struct Environment {
    };

    /**
     * This function is set up to be called when the SIGINT signal is
     * received by the program.  It just sets the "shutDown" flag
     * and relies on the program to be polling the flag to detect
     * when it's been set.
     *
     * @param[in] sig
     *     This is the signal for which this function was called.
     */
    void InterruptHandler(int) {
        shutDown = true;
    }

    /**
     * This function updates the program environment to incorporate
     * any applicable command-line arguments.
     *
     * @param[in] argc
     *     This is the number of command-line arguments given to the program.
     *
     * @param[in] argv
     *     This is the array of command-line arguments given to the program.
     *
     * @param[in,out] environment
     *     This is the environment to update.
     *
     * @param[in] diagnosticMessageDelegate
     *     This is the function to call to publish any diagnostic messages.
     *
     * @return
     *     An indication of whether or not the function succeeded is returned.
     */
    bool ProcessCommandLineArguments(
        int argc,
        char* argv[],
        Environment& environment,
        const SystemAbstractions::DiagnosticsSender& diagnosticMessageSender
    ) {
        bool emailFileNameSet = false;
        size_t state = 0;
        for (int i = 1; i < argc; ++i) {
            const std::string arg(argv[i]);
            switch (state) {
                case 0: { // extra argument
                    diagnosticMessageSender.SendDiagnosticInformationString(
                        SystemAbstractions::DiagnosticsSender::Levels::ERROR,
                        "extra arguments given"
                    );
                    return false;
                } break;
            }
        }
        return true;
    }

    bool StartWebServer(
        Http::Server& http,
        const SystemAbstractions::DiagnosticsSender& diagnosticMessageSender
    ) {
        Http::Server::MobilizationDependencies httpDeps;
        httpDeps.timeKeeper = std::make_shared< TimeKeeper >();
        auto transport = std::make_shared< HttpNetworkTransport::HttpServerNetworkTransport >();
        transport->SubscribeToDiagnostics(diagnosticMessageSender.Chain(), 0);
        httpDeps.transport = transport;
        http.SetConfigurationItem("Port", "8080");
        return http.Mobilize(httpDeps);
    }

    void RegisterTestResource(
        Http::Server& http
    ) {
        http.RegisterResource(
            {"on"},
            [](
                const Http::Request& request,
                std::shared_ptr< Http::Connection > connection,
                const std::string& trailer
            ){
                const auto params = StringExtensions::Split(
                    request.target.GetQuery(),
                    "&"
                );
                uint8_t brightness = 0;
                uint8_t red = 0;
                uint8_t green = 0;
                uint8_t blue = 0;
                std::ostringstream buf;
                buf << "Query: " << request.target.GetQuery() << "\r\n";
                for (const auto& param: params) {
                    const auto keyValue = StringExtensions::Split(param, "=");
                    if (keyValue.size() < 2) {
                        continue;
                    }
                    const auto& key = keyValue[0];
                    const auto& value = keyValue[1];
                    buf << "  " << key << " = " << value << "\r\n";
                    if (key == "b") {
                        (void)sscanf(value.c_str(), "%" SCNu8, &brightness);
                        buf << "brightness: " << (int)brightness << "\r\n";
                    } else if (key == "c") {
                        (void)sscanf(
                            value.c_str(),
                            "%2" SCNx8 "%2" SCNx8 "%2" SCNx8,
                            &red,
                            &green,
                            &blue
                        );
                        buf << "red: " << (int)red << "\r\n";
                        buf << "green: " << (int)green << "\r\n";
                        buf << "blue: " << (int)blue << "\r\n";
                    }
                }
                Leds::TurnOn(brightness, red, green, blue);
                Http::Response response;
                response.statusCode = 200;
                response.reasonPhrase = "OK";
                response.headers.SetHeader("Content-Type", "text/plain");
                response.body = buf.str();
                return response;
            }
        );
        http.RegisterResource(
            {"off"},
            [](
                const Http::Request& request,
                std::shared_ptr< Http::Connection > connection,
                const std::string& trailer
            ){
                Leds::TurnOff();
                Http::Response response;
                response.statusCode = 200;
                response.reasonPhrase = "OK";
                response.headers.SetHeader("Content-Type", "text/plain");
                response.body = "BibleThump\r\n";
                return response;
            }
        );
    }

}

/**
 * This function is the entrypoint of the program.
 * It just sets up the web client, using it to fetch a resource
 * and generate a report.  It registers the SIGINT signal to know
 * when the web client should be shut down early.
 *
 * The program is terminated after either a report is generated
 * or the SIGINT signal is caught.
 *
 * @param[in] argc
 *     This is the number of command-line arguments given to the program.
 *
 * @param[in] argv
 *     This is the array of command-line arguments given to the program.
 */
int main(int argc, char* argv[]) {
#ifdef _WIN32
    //_crtBreakAlloc = 18;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif /* _WIN32 */
    const auto previousInterruptHandler = signal(SIGINT, InterruptHandler);
    Environment environment;
    (void)setbuf(stdout, NULL);
    const auto diagnosticsPublisher = SystemAbstractions::DiagnosticsStreamReporter(stdout, stderr);
    SystemAbstractions::DiagnosticsSender diagnosticsSender("Lighter");
    diagnosticsSender.SubscribeToDiagnostics(diagnosticsPublisher);
    if (!ProcessCommandLineArguments(argc, argv, environment, diagnosticsSender)) {
        PrintUsageInformation();
        return EXIT_FAILURE;
    }
    Http::Server http;
    if (!StartWebServer(http, diagnosticsSender)) {
        fprintf(stderr, "Error: unable to set up web server!\n");
        return EXIT_FAILURE;
    }
    RegisterTestResource(http);
    printf("Web server active!  Press <Ctrl>+<C> (and possibly <Enter> as well) to quit.\n");
    while (!shutDown) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    printf("kthxbye!\n");
    (void)signal(SIGINT, previousInterruptHandler);
    return EXIT_SUCCESS;
}
