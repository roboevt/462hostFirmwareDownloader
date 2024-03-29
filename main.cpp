#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include "Richarduino.h"

int parseArgs(int argc, char** argv, std::string& firmwarePath, std::string& richarduinoPort) {
    if (argc > 1) {
        firmwarePath = argv[1];
    }
    if (argc > 2) {
        richarduinoPort = argv[2];
    }
    if (argc > 3) {
        std::cout << "Usage: " << argv[0] << " [firmware.bin] [richarduino port]" << std::endl;
        return 1;
    }
    return 0;
}

std::vector<uint32_t> readFirmwareFile(std::string firmwarePath) {
    std::ifstream firmwareFile(firmwarePath);

    uint32_t address;
    firmwareFile >> std::hex >> address;
    if (address != 4096) {
        std::cout << "Invalid firmware file, first address must be 4096, was " +
                         std::to_string(address)
                  << std::endl;
        return {};
    }

    std::vector<uint32_t> firmware;

    std::cout << "Reading firmware from " << firmwarePath << ". Firmware: " << std::endl;

    hex(std::cout);
    while (firmwareFile.good()) {
        uint32_t instruction;
        if (firmwareFile >> address >> instruction) {
            firmware.push_back(instruction);

            std::cout << address << ": " << instruction << std::endl;
        }
    }
    dec(std::cout);

    return firmware;
}

auto main(int argc, char** argv) -> int {
    std::string firmwarePath = "firmware.bin";
    std::string richarduinoPort = "/dev/ttyUSB1";
    if (parseArgs(argc, argv, firmwarePath, richarduinoPort)) {
        return 1;
    }

    Richarduino richarduino(richarduinoPort, B115200);

    std::vector<uint32_t> firmware = readFirmwareFile(firmwarePath);

    richarduino.program(firmware);

}