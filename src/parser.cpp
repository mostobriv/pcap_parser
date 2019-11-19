#include "parser.h"

#include <iostream>
#include <stdexcept>
#include <utility>
#include <fmt/format.h>


PcapLoader::PcapLoader(size_t cache_size) :
            autoremove(false),
            connection_manager(cache_size),
            cleanup_configuration(true, 1, 50),
            reassembler(on_message_ready_callback, 
                        &connection_manager,
                        on_connection_start_callback, 
                        on_connection_end_callback,
                        cleanup_configuration),
            _logger("PcapLoader")
{
    _logger.debug(__PRETTY_FUNCTION__);
}


bool PcapLoader::parse(std::string filename) {
    _logger.debug(__PRETTY_FUNCTION__);

    RawPacket rawPacket;
    auto reader = std::unique_ptr<IFileReaderDevice>(IFileReaderDevice::getReader(filename.c_str()));

    if (!reader->open()) {
        throw std::invalid_argument(fmt::format("Can't open file: {}", filename));
    }

    while (reader->getNextPacket(rawPacket)) {
		reassembler.reassemblePacket(&rawPacket);
	}

	auto processed_connections = reassembler.getConnectionInformation().size();
    _logger.info(fmt::format("Processed {} connections", processed_connections));

    reassembler.closeAllConnections();
    reader->close();

    return true;
}


PcapLoader::~PcapLoader() {
    _logger.debug(__PRETTY_FUNCTION__);
}


std::string buf;
void PcapLoader::on_message_ready_callback(int side, TcpStreamData tcp_data, void* user_cookie) {

    auto manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(tcp_data.getConnectionDataRef().flowKey);

    // if connection not in the map yet (idk how it's even possible but who the fuck cares about)
    auto& conn_data = tcp_data.getConnectionDataRef();
    if (manager_iter == manager->table.end()) {
		manager->table.insert({tcp_data.getConnectionDataRef().flowKey, reassembly_state_t(conn_data.srcPort, conn_data.dstPort)});
		manager_iter = manager->table.find(tcp_data.getConnectionDataRef().flowKey);
	}

    auto& conn_state = manager_iter->second;

    if (conn_state.msg_count == 0 || conn_state.current_side != side) {
        conn_state.current_side = static_cast<Side>(side);
        if (conn_state.msg_count != 0) {
            std::cout << "=============================== ! side ! ===============================" << std::endl << buf;
        }
        if (buf.back() != '\n') {
            std::cout << std::endl;
        }
        buf.clear();
    }
    conn_state.msg_count++;
    
    buf+= reinterpret_cast<char*>(tcp_data.getData());
}


void PcapLoader::on_connection_start_callback(ConnectionData connection_data, void* user_cookie) {

    auto manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(connection_data.flowKey);

    // new connection
    if (manager_iter == manager->table.end()) {
        manager->table.insert({connection_data.flowKey, reassembly_state_t(connection_data.srcPort, connection_data.dstPort)});
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