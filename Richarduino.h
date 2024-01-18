#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct Richarduino {
    int portFd;
    struct termios tty;
    int baud;

    Richarduino(std::string location, int baud) : baud(baud) {
        // Open serial port connection
        std::cout << "Opening serial port " << location << std::endl;
        portFd = open(location.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (portFd < 0) {
            throw std::runtime_error("Failed to open serial port");
        }
        std::cout << "Got port fd " << portFd << std::endl;
        // Edit settings
        if (tcgetattr(portFd, &tty) != 0) { /* handle error */
            abort();
        }
        // Set baud rate
        cfsetispeed(&tty, baud);
        cfsetospeed(&tty, baud);
        // cfsetspeed(&tty, baud);

        tty.c_cc[VTIME] = 5;
        tty.c_cc[VMIN] = 0;
        tty.c_lflag = 0;

        if (tcsetattr(portFd, TCSANOW, &tty) != 0) { /* handle error */
            abort();
        }
    }

    ~Richarduino() {
        std::cout << "Closing serial port" << std::endl;
        close(portFd);
    }

    void write(std::string data) {
        int num_written = ::write(portFd, "V", 1);
        if (num_written < 0) { /* handle error */
            abort();
        }
        std::cout << "Wrote " << num_written << " bytes" << std::endl;
    }

    std::string read(int n) {
        std::vector<char> data(n);
        std::cout << "Before read" << std::endl;
        int num_read = ::read(portFd, data.data(), n);
        std::cout << "After read" << std::endl;
        // Processing
        if (num_read == n) {
            return std::string(data.begin(), data.end());
        }
        std::cout << "Read " << num_read << " bytes, expected " << n << std::endl;
        return "";
    }
};