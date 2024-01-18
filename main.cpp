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

std::vector<int32_t> readFirmware(std::string firmwarePath) {
    std::ifstream firmwareFile(firmwarePath);

    int address;
    firmwareFile >> std::hex >> address;
    if (address != 4096) {
        std::cout << "Invalid firmware file, first address must be 4096, was " +
        std::to_string(address) << std::endl; return {};
    }

    std::vector<int32_t> firmware;

    while (firmwareFile.good()) {
        int instruction;
        firmwareFile >> address >> instruction;
        firmware.push_back(instruction);

        std::cout << address << " " << instruction << std::endl;
    }

    return firmware;
}

int uploadFirmware(std::vector<int32_t> firmware, Richarduino& richarduino) {
    std::cout << "Uploading firmware..." << std::endl;

    int programLength = firmware.size() * 4;  // 4 bytes per instruction

    std::vector<char> programLengthBytes(4);
    programLengthBytes[0] = (programLength >> 24) & 0xFF;
    programLengthBytes[1] = (programLength >> 16) & 0xFF;
    programLengthBytes[2] = (programLength >> 8) & 0xFF;
    programLengthBytes[3] = programLength & 0xFF;
    std::string programLengthString(programLengthBytes.begin(), programLengthBytes.end());

    // Program command
    richarduino.write("P\n");

    // Length
    std::cout << "Sending length " << programLengthString << std::endl;
    richarduino.write(std::string(programLengthString));

    // Program instructions
    for (int32_t instruction : firmware) {
        std::vector<char> instructionBytes(4);
        instructionBytes[0] = (instruction >> 24) & 0xFF;
        instructionBytes[1] = (instruction >> 16) & 0xFF;
        instructionBytes[2] = (instruction >> 8) & 0xFF;
        instructionBytes[3] = instruction & 0xFF;
        std::string instructionString(instructionBytes.begin(), instructionBytes.end());

        std::cout << std::hex << "Sending instruction " << instructionString << std::endl;
        richarduino.write(std::string(instructionString));
    }

    return 0;
}

auto main(int argc, char** argv) -> int {
    std::string firmwarePath = "firmware.bin";
    std::string richarduinoPort = "/dev/ttyUSB1";
    // if (parseArgs(argc, argv, firmwarePath, richarduinoPort)) {
    //     return 1;
    // }

    Richarduino richarduino(richarduinoPort, B115200);
    richarduino.write("V");
    std::cout << "Firmware version: " << richarduino.read(1) << std::endl;

    std::vector<int32_t> firmware = readFirmware(firmwarePath);

    // uploadFirmware(firmware, richarduino);

}