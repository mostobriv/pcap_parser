#include "parser.h"
#include "logger.h"

#include <iostream>



int main(int argc, char** argv) {

    PcapLoader foo;
    try {
        foo.parse(argv[1]);

    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}