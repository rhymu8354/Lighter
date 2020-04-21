#include "../Leds.hpp"

#include <fcntl.h>
#include <functional>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <memory>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

namespace {

    bool SetLeds(
        uint8_t brightness,
        uint8_t red,
        uint8_t green,
        uint8_t blue
    ) {
        const uint8_t mode = 0;
        const uint8_t bits = 8;
        const uint32_t speed = 500000;

        std::vector< uint8_t > tx(4 + 144*4 + 12);
        for (size_t i = 0; i < 144; ++i) {
            tx[i * 4 + 4] = (0xE0 | brightness);
            tx[i * 4 + 5] = blue;
            tx[i * 4 + 6] = green;
            tx[i * 4 + 7] = red;
        }
        std::vector< uint8_t > rx(tx.size());
        struct spi_ioc_transfer tr;
        tr.tx_buf = (unsigned long)tx.data();
        tr.rx_buf = (unsigned long)rx.data();
        tr.len = tx.size();
        tr.delay_usecs = 0;
        tr.speed_hz = speed;
        tr.bits_per_word = bits;

        int fd = open("/dev/spidev0.0", O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "error opening SPI device\n");
            return false;
        }

        /*
         * use a unique_ptr to auto-close the file upon return
         */
        std::unique_ptr< int, std::function< void(int* fd) > > fdCloser(
            &fd,
            [](int* fd){ close(*fd); }
        );

        /*
         * set SPI mode
         */
        int ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1) {
            fprintf(stderr, "error setting SPI mode\n");
            return false;
        }

        /*
         * set bits per word
         */
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1) {
            fprintf(stderr, "error setting SPI bits per word\n");
            return false;
        }

        /*
         * set max speed (in Hertz)
         */
        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1) {
            fprintf(stderr, "error setting SPI max speed\n");
            return false;
        }

        /*
         * push data out the SPI to the light strip
         */
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1) {
            fprintf(stderr, "error sending SPI data\n");
            return false;
        }
        return true;
    }

}

namespace Leds {

    void TurnOn() {
        if (!SetLeds(4, 0xFF, 0x22, 0x22)) {
            fprintf(stderr, "FeelsBadMan\n");
        }
    }

    void TurnOff() {
        if (!SetLeds(0, 0, 0, 0)) {
            fprintf(stderr, "FeelsBadMan\n");
        }
    }

}
