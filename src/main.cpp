#include <thread>
#include <csignal>
#include <functional>
#include "inotify-cpp/NotifierBuilder.h"

#include "StreamData.h"
#include "ThreadQueue.hpp"
#include "PcapLoader.h"
#include "DatabaseWriter.h"
#include "logger.h"
#include "RunningStatus.h"
#include "Configuration.h"


void set_sigint_handler(std::function<void()>&&);
inotify::NotifierBuilder create_notifier
    ( const std::vector<std::filesystem::path>& paths
    , std::function<void(inotify::Notification)>&&
    );


int main(int argc, const char** argv)
{
    auto conf = Configuration(argc, argv);
    if (conf.should_exit) {
        return 0;
    }

    auto db_creds = conf.db_creds.value_or(DatabaseWriter::DefaultConnData);

    ThreadQueue<StreamData> data_queue;
    ThreadQueue<std::string> file_queue;

    // add all files provided on command line
    for (const auto& file : conf.files) {
        file_queue.emplace(file);
        logger::logger.debug() << "add file" << file;
    }

    try {
        // start real workers
        auto loader = PcapLoader(data_queue, file_queue);
        auto writer = DatabaseWriter(data_queue, db_creds);
        auto notifier =
            create_notifier(conf.dirs, [&] (inotify::Notification n) {
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

        logger::logger.info() << "All worker threads are up."
            << "Press ctrl-c to stop gracefully";

        // set handler for graceful stopping
        RunningStatus threads_status = RunningStatus::Run;
        set_sigint_handler([&] {
            if (threads_status == RunningStatus::Run) {
                threads_status = RunningStatus::StopWhenEmpty;
                std::cout << "\b\bGracefully stopping."
                    << " Press again to speed up."
                    << std::endl;

                // Stop watching directory
                notifier.stop();
                // Parse remaining files
                loader.set_should_stop(RunningStatus::StopWhenEmpty);
                while (not data_queue.empty()) {
                    // Wait to finish parsing
                    std::this_thread::yield();
                }
                // Load remaining streams
                writer.set_should_stop(RunningStatus::StopWhenEmpty);
            } else if (threads_status == RunningStatus::StopWhenEmpty) {
                threads_status = RunningStatus::StopNow;
                loader.set_should_stop(RunningStatus::StopNow);
                writer.set_should_stop(RunningStatus::StopNow);
                std::cout << "\b\bPress again to abort immediately" << std::endl;
            } else if (threads_status == RunningStatus::StopNow) {
                exit(1);
                std::cout << "It looks like exit(1) failed."
                    << " Kill the process named " << conf.program_name
                    << " with OS methods."
                    << std::endl;
            }
        });

        notifier_thread.join();
        loader_thread.join();
        writer_thread.join();

    } catch (const std::exception &e) {
        logger::logger.error() << e.what();
    }

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
    ( const std::vector<std::filesystem::path>& paths
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
