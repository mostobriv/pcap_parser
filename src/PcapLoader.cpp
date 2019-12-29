#include "PcapLoader.h"

#include <iostream>
#include <stdexcept>

#include <PcapLiveDeviceList.h>
#include <PcapFileDevice.h>
#include <PlatformSpecificUtils.h>
#include <SystemUtils.h>
#include <PcapPlusPlusVersion.h>


logger::Logger PcapLoader::logger ("Pcap");


using Side = StreamData::Side;


PcapLoader::PcapLoader(ThreadQueue<StreamData>& q, size_t cache_size)
    : connection_manager(cache_size, q)
    , cleanup_configuration(true, 1, 50)
    , reassembler( on_message_ready_callback
                 , &connection_manager
                 , on_connection_start_callback
                 , on_connection_end_callback
                 , cleanup_configuration
                 )
{
    logger.debug(__PRETTY_FUNCTION__);
}


void PcapLoader::parse(const std::string& filename)
{
    logger.debug(__PRETTY_FUNCTION__);

    pcpp::RawPacket rawPacket;
    auto reader = std::unique_ptr<pcpp::IFileReaderDevice>(
            pcpp::IFileReaderDevice::getReader(filename.c_str())
            );

    if (!reader->open()) {
        throw std::invalid_argument("Can't open file: " + filename);
    }

    while (reader->getNextPacket(rawPacket)) {
        reassembler.reassemblePacket(&rawPacket);
    }

    const auto processed_connections = reassembler.getConnectionInformation().size();
    logger.info() << "Processed" << processed_connections << "connections";

    reassembler.closeAllConnections();
    reader->close();
}


PcapLoader::~PcapLoader()
{
    logger.debug(__PRETTY_FUNCTION__);
}


void PcapLoader::on_message_ready_callback(
          int side, pcpp::TcpStreamData tcp_data
        , void* user_cookie
        )
{
    logger.debug(__PRETTY_FUNCTION__);

    auto* const manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(tcp_data.getConnectionData().flowKey);

    // if connection not in the map yet (idk how it's even possible but who the
    // fuck cares about)
    auto& conn_data = tcp_data.getConnectionData();
    if (manager_iter == manager->table.end()) {
        auto state = reassembly_state_t(conn_data.srcPort, conn_data.dstPort);
        manager->table.emplace(tcp_data.getConnectionData().flowKey, state);
        manager_iter = manager->table.find(tcp_data.getConnectionData().flowKey);
    }

    auto& conn_state = manager_iter->second;
    auto& buf = conn_state.buffer;

    if (conn_state.msg_count == 0) {
        conn_state.current_side = static_cast<Side>(side);
        conn_state.all_data.start_side = conn_state.current_side;
    } else if (conn_state.current_side != side) {
        conn_state.current_side = static_cast<Side>(side);

        conn_state.all_data.push_back(buf);
        buf.clear();
    }
    conn_state.msg_count++;

    auto* raw_data = reinterpret_cast<char*>(tcp_data.getData());
    auto  data_len = tcp_data.getDataLength();
    buf += std::string(raw_data, data_len);
}


void PcapLoader::on_connection_start_callback(
          pcpp::ConnectionData connection_data
        , void* user_cookie
        )
{
    logger.debug(__PRETTY_FUNCTION__);

    auto manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(connection_data.flowKey);

    // new connection
    if (manager_iter == manager->table.end()) {
        auto state = reassembly_state_t(
                  connection_data.srcPort
                , connection_data.dstPort
                );
        manager->table.emplace(connection_data.flowKey, state);
    }
}


void PcapLoader::on_connection_end_callback(
          pcpp::ConnectionData connection_data
        , pcpp::TcpReassembly::ConnectionEndReason reason
        , void* user_cookie
        )
{
    logger.debug(__PRETTY_FUNCTION__);

    auto manager = reinterpret_cast<conn_mgr_t*>(user_cookie);
    auto manager_iter = manager->table.find(connection_data.flowKey);

    // kind of mistake??
    if (manager_iter == manager->table.end()) {
        return;
    }

    auto& conn_state = manager_iter->second;
    manager->queue.push(std::move(conn_state.all_data));
    manager->table.erase(manager_iter);
}
