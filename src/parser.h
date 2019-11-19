#pragma once


#include "logger.h"

#include <map>

#include <TcpReassembly.h>
#include <PcapLiveDeviceList.h>
#include <PcapFileDevice.h>
#include <PlatformSpecificUtils.h>
#include <SystemUtils.h>
#include <PcapPlusPlusVersion.h>
#include <LRUList.h>

using namespace pcpp;


enum Side {client = 0, server, unknown};


struct reassembly_state_t {
    Side current_side;
    uint32_t msg_count;
    uint16_t src_port;
    uint16_t dest_port;


    reassembly_state_t() : reassembly_state_t(unknown, 0, 0, 0) {};
    reassembly_state_t(uint16_t s, uint16_t d) : reassembly_state_t(unknown, 0, s, d) {};
    reassembly_state_t(Side start_side, uint32_t msg_count, uint16_t _src_port, uint16_t _dest_port) {
        current_side    = start_side;
        msg_count       = msg_count;
        src_port        = _src_port;
        dest_port       = _dest_port;

    }
    
    ~reassembly_state_t() {}
};


using conn_table = std::map<uint32_t, reassembly_state_t>;
using conn_table_iter = std::map<uint32_t, reassembly_state_t>::iterator;


struct conn_mgr_t {
    conn_table table;
    LRUList<uint32_t> cache;


    conn_mgr_t(size_t cache_size) : table(), cache(cache_size) {}
    ~conn_mgr_t() {}
};


class PcapLoader {
    private:

        bool autoremove;

        conn_mgr_t connection_manager;
        TcpReassemblyConfiguration cleanup_configuration;
        TcpReassembly reassembler;
        
        static void on_message_ready_callback(int side, TcpStreamData tcp_data, void* user_cookie);
        static void on_connection_start_callback(ConnectionData connection_data, void* user_cookie);
        static void on_connection_end_callback(ConnectionData connection_data, TcpReassembly::ConnectionEndReason reason, void* user_cookie);


    public:

        Logger _logger;

        PcapLoader(size_t cache_size=64);
        // Watch();
        bool parse(std::string filename);
        ~PcapLoader();

        bool get_autoremove() { return autoremove; };
        void set_autoremove(bool state) { autoremove = state; };

};