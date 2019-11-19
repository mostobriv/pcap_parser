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


    reassembly_state_t() : current_side(unknown), msg_count(0) {}
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