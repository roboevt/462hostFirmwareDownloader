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
        // Edit settings
        if (tcgetattr(portFd, &tty) != 0) { /* handle error */
            abort();
        }
        // Set baud rate
        cfsetispeed(&tty, baud);
        cfsetospeed(&tty, baud);

        if (tcsetattr(portFd, TCSANOW, &tty) != 0) { /* handle error */
            abort();
        }
    }

    ~Richarduino() {
        std::cout << "Closing serial port" << std::endl;
        close(portFd);
    }

    void write(std::string data) {
        int num_written = ::write(portFd, data.c_str(), data.size());
        if (num_written < 0) { /* handle error */
            abort();
        }
    }

    void write(int32_t data) {
        std::vector<char> dataBytes(4);
        dataBytes[0] = (data >> 24) & 0xFF;
        dataBytes[1] = (data >> 16) & 0xFF;
        dataBytes[2] = (data >> 8) & 0xFF;
        dataBytes[3] = data & 0xFF;
        write(std::string(dataBytes.begin(), dataBytes.end()));
    }

    std::string read(int n) {
        std::vector<char> data(n);
        int num_read = ::read(portFd, data.data(), n);
        // Processing
        if (num_read == n) {
            return std::string(data.begin(), data.end());
        }
        std::cout << "Error: Read " << num_read << " bytes, expected " << n << std::endl;
        return "";
    }

    void program(std::vector<int32_t> program) {
        std::cout << "Uploading program..." << std::endl;

        int programLength = program.size() * 4;  // 4 bytes per instruction

        std::vector<char> programLengthBytes(4);
        programLengthBytes[0] = (programLength >> 24) & 0xFF;
        programLengthBytes[1] = (programLength >> 16) & 0xFF;
        programLengthBytes[2] = (programLength >> 8) & 0xFF;
        programLengthBytes[3] = programLength & 0xFF;
        std::string programLengthString(programLengthBytes.begin(), programLengthBytes.end());

        // Program command
        write("P");
        write(programLengthString);

        // Program data
        for (int32_t instruction : program) {
            write(instruction);
        }
    }

    void poke(int32_t addr, int32_t data) {
        std::string pokeCommand = "W";
        write(pokeCommand);
        write(addr);
        write(data);
    }

    int peek(int32_t addr) {
        std::string peekCommand = "R";
        write(peekCommand);
        write(addr);
        std::string response = read(4);
        int responseInt = 0;
        responseInt |= response[0] << 24;
        responseInt |= response[1] << 16;
        responseInt |= response[2] << 8;
        responseInt |= response[3];
        return responseInt;
    }
};