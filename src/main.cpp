#include "parser.h"
#include "logger.h"



int main(int argc, char** argv) {

    if (argc < 2) {
        logger::logger.error() << "Usage:" << argv[0] << "[*].pcap";
        return 1;
    }

    PcapLoader foo;
    try {
        foo.parse(argv[1]);

    } catch (const std::exception& e) {
        logger::logger.error() << e.what();
    }

    return 0;
}
