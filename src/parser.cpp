#include "parser.h"

#include <iostream>
#include <stdexcept>
#include <utility>
#include <fmt/format.h>


// PcapLoader::PcapLoader() : PcapLoader::PcapLoader(64) {};

PcapLoader::PcapLoader(size_t cache_size) :
            connection_manager(cache_size),
            cleanup_configuration(true, 1, 50),
            reassembler(on_message_ready_callback, 
                        &connection_manager,
                        on_connection_start_callback, 
                        on_connection_end_callback,
                        cleanup_configuration)
{
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

}

template <class E>
void throw_with_trace(const E& e) {
    throw boost::enable_error_info(e)
        << traced(boost::stacktrace::stacktrace());
}


bool PcapLoader::parse(std::string filename) {
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

    RawPacket rawPacket;
    auto reader = std::unique_ptr<IFileReaderDevice>(IFileReaderDevice::getReader(filename.c_str()));

    if (!reader->open()) {
        throw_with_trace(
            std::invalid_argument(fmt::format("Can't open file in reassembler: {}", filename))
        );
    }

    while (reader->getNextPacket(rawPacket)) {
		reassembler.reassemblePacket(&rawPacket);
	}

#ifdef DEBUG
	auto processed_connections = reassembler.getConnectionInformation().size();
    std::cout << "[*] Processed " << processed_connections << " connections" << std::endl;
#endif

    reassembler.closeAllConnections();
    reader->close();

    return true;
}


PcapLoader::~PcapLoader() {
#ifdef DEBUG
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}


void PcapLoader::on_message_ready_callback(int side, TcpStreamData tcp_data, void* user_cookie) {

    auto manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(tcp_data.getConnectionDataRef().flowKey);

    if (manager_iter == manager->table.end()) {
		manager->table.insert({tcp_data.getConnectionDataRef().flowKey, std::move(reassembly_state_t())});
		manager_iter = manager->table.find(tcp_data.getConnectionDataRef().flowKey);
	}
}


void PcapLoader::on_connection_start_callback(ConnectionData connection_data, void* user_cookie) {

    auto manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(connection_data.flowKey);

    // new connection
    if (manager_iter == manager->table.end()) {
        manager->table.insert({connection_data.flowKey, std::move(reassembly_state_t())});
    }
}


void PcapLoader::on_connection_end_callback(ConnectionData connection_data, TcpReassembly::ConnectionEndReason reason, void* user_cookie) {

    auto manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(connection_data.flowKey);

    // kind of mistake??
    if (manager_iter == manager->table.end()) {
        return;
    }


    manager->table.erase(manager_iter);
}