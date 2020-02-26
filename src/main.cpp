#include <thread>
#include <csignal>
#include <functional>

#include "StreamData.h"
#include "ThreadQueue.hpp"
#include "PcapLoader.h"
#include "DatabaseWriter.h"
#include "logger.h"


void set_sigint_handler(std::function<void()>&&);


int main(int argc, char** argv)
{
    ThreadQueue<StreamData> queue;

    if (argc < 2) {
        logger::logger.error() << "Usage:" << argv[0] << "[*].pcap";
        return 1;
    }

    try {
        // start real workers
        auto loader = PcapLoader(queue);
        auto writer = DatabaseWriter(queue);

        auto loader_thread = std::thread([&] {
            loader.parse(argv[1]);
        });
        auto writer_thread = std::thread([&] {
            writer.write();
        });

        // set handler for graceful stopping
        bool ctrlc_pressed = false;
        set_sigint_handler([&] {
            if (ctrlc_pressed) {
                logger::logger.warning() << "Stopping immediately";
                exit(1);
            } else {
                ctrlc_pressed = true;
                logger::logger.warning() << "Press again to stop immediately";
            }
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


std::function<void()> global_signal_handler = [](){};
void signal_handler(int)
{
    global_signal_handler();
}

void set_sigint_handler(std::function<void()>&& handler)
{
    global_signal_handler = handler;

    struct sigaction sig_int_handler;
    sig_int_handler.sa_handler = signal_handler;
    sigemptyset(&sig_int_handler.sa_mask);
    sig_int_handler.sa_flags = 0;
    sigaction(SIGINT, &sig_int_handler, NULL);
}
