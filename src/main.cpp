#include "StreamData.h"
#include "ThreadQueue.hpp"
#include "PcapLoader.h"
#include "DatabaseWriter.h"
#include "logger.h"


int main(int argc, char** argv)
{
    try {
        ThreadQueue<StreamData> queue;
        auto w = DatabaseWriter(queue);
        return 0;

    } catch (const std::exception &e) {
        logger::logger.error() << e.what();
    }

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
