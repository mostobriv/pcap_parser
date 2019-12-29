#include "StreamData.h"
#include "ThreadQueue.hpp"
#include "PcapLoader.h"
#include "DatabaseWriter.h"
#include "logger.h"


int main(int argc, char** argv)
{
    ThreadQueue<StreamData> queue;

    try {
        auto w = DatabaseWriter(queue);
        return 0;

    } catch (const std::exception &e) {
        logger::logger.error() << e.what();
    }

    if (argc < 2) {
        logger::logger.error() << "Usage:" << argv[0] << "[*].pcap";
        return 1;
    }

    auto loader = PcapLoader(queue);
    try {
        loader.parse(argv[1]);

    } catch (const std::exception& e) {
        logger::logger.error() << e.what();
    }

    while (not queue.empty()) {
        auto stream = queue.pop();
        auto side = stream.start_side;
        std::cout << "+++++ conversation +++++\n";
        for (const auto& reply : stream.data()) {
            std::cout << "===== " << std::to_string(side) << " =====\n"
                      << reply << std::endl;
            side = StreamData::flip_side(side);
        }
    }

    return 0;
}
