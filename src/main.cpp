#include <thread>

#include "StreamData.h"
#include "ThreadQueue.hpp"
#include "PcapLoader.h"
#include "DatabaseWriter.h"
#include "logger.h"


int main(int argc, char** argv)
{
    ThreadQueue<StreamData> queue;

    if (argc < 2) {
        logger::logger.error() << "Usage:" << argv[0] << "[*].pcap";
        return 1;
    }

    try {
        auto loader = PcapLoader(queue);
        auto writer = DatabaseWriter(queue);

        auto loader_thread = std::thread([&] {
            loader.parse(argv[1]);
        });

        auto writer_thread = std::thread([&] {
            writer.write();
        });

        loader_thread.join();
        while (not queue.empty()) {
            // wait for the writer to get all values
            std::this_thread::yield();
        }
        writer.set_should_stop(true);
        writer_thread.join();

    } catch (const std::exception &e) {
        logger::logger.error() << e.what();
    }

    /*
     * may be used again later
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
    */

    return 0;
}
