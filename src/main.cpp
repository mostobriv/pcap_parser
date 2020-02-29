#include <thread>
#include <csignal>
#include <functional>
#include <boost/filesystem.hpp>
#include "inotify-cpp/NotifierBuilder.h"

#include "StreamData.h"
#include "ThreadQueue.hpp"
#include "PcapLoader.h"
#include "DatabaseWriter.h"
#include "logger.h"


void set_sigint_handler(std::function<void()>&&);
inotify::NotifierBuilder create_notifier
    ( const std::vector<boost::filesystem::path>& paths
    , std::function<void(inotify::Notification)>&&
    );


int main(int argc, const char** argv)
{
    ThreadQueue<StreamData> data_queue;
    ThreadQueue<std::string> file_queue;

    if (argc < 2) {
        logger::logger.error() << "Usage:" << argv[0] << "[*].pcap";
        return 1;
    }

    // add all files provided on command line
    for (int i = 1; i < argc; ++i) {
        file_queue.emplace(argv[i]);
        logger::logger.debug() << "add file" << argv[i];
    }

    try {
        // start real workers
        auto loader = PcapLoader(data_queue, file_queue);
        auto writer = DatabaseWriter(data_queue);
        auto notifier = create_notifier({"./pcaps"}, [&] (inotify::Notification n) {
            if (n.path.extension() == ".pcap") {
                file_queue.emplace(n.path.generic_string());
                logger::logger.info() << "Add new file:" << n.path;
            }
        });

        auto loader_thread = std::thread([&] {
            loader.start_parsing();
        });
        auto writer_thread = std::thread([&] {
            writer.start_writing();
        });
        auto notifier_thread = std::thread([&] {
            notifier.run();
        });

        // set handler for graceful stopping
        bool ctrlc_pressed = false;
        set_sigint_handler([&] {
            if (ctrlc_pressed) {
                std::cout << "\b\b";
                logger::logger.info() << "Stopping immediately";
                exit(1);
            } else {
                ctrlc_pressed = true;
                std::cout << "\b\bGracefully stopping. Press again to stop immediately"
                          << std::endl;
                notifier.stop();
                writer.set_should_stop();
                loader.set_should_stop();
            }
        });

        // TODO: rework waiting

        loader_thread.join();

        while (not data_queue.empty()) {
            // wait for the writer to get all values
            std::this_thread::yield();
        }

        writer.set_should_stop();
        writer_thread.join();

        notifier_thread.join();

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


inotify::NotifierBuilder create_notifier
    ( const std::vector<boost::filesystem::path>& paths
    , std::function<void(inotify::Notification)>&& cb
    )
{
    auto events = { inotify::Event::close_write
                  , inotify::Event::moved_to
                  };
    auto notifier = inotify::BuildNotifier()
        .onEvents(events, cb)
        ;
    for (const auto& path : paths) {
        notifier.watchPathRecursively(path);
    }
    return notifier;
}
