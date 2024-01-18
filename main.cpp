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
                         std::to_string(address)
                  << std::endl;
        return {};
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

auto main(int argc, char** argv) -> int {
    std::string firmwarePath = "firmware.bin";
    std::string richarduinoPort = "/dev/ttyUSB1";
    if (parseArgs(argc, argv, firmwarePath, richarduinoPort)) {
        return 1;
    }

    Richarduino richarduino(richarduinoPort, B115200);
    richarduino.write("V");
    std::cout << "Firmware version: " << richarduino.read(1) << std::endl;

    for(int i = 0; i < 128; i+= 4) {
        std::cout << "Peek " << std::hex << i << ": " << richarduino.peek(i) << std::endl;
    }

    std::cout << "Peek 4096: " << richarduino.peek(4096) << std::endl;
    richarduino.poke(4096, 2);
    std::cout << "Peek 4096: " << richarduino.peek(4096) << std::endl;
    richarduino.poke(4096, -1);
    std::cout << "Peek 4096: " << richarduino.peek(4096) << std::endl;

    std::vector<int32_t> firmware = readFirmware(firmwarePath);

    richarduino.program(firmware);
}