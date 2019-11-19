#pragma once

#include <map>

#include <TcpReassembly.h>
#include <PcapLiveDeviceList.h>
#include <PcapFileDevice.h>
#include <PlatformSpecificUtils.h>
#include <SystemUtils.h>
#include <PcapPlusPlusVersion.h>
#include <LRUList.h>

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

using namespace pcpp;


#include <iostream>

enum Side {client = 0, server};


typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;


typedef struct reassembly_state {
    Side current_side;


    reassembly_state() : current_side(client) {}
    ~reassembly_state() {}
} reassembly_state_t;


typedef std::map<uint32_t, reassembly_state_t> conn_table;
typedef std::map<uint32_t, reassembly_state_t>::iterator conn_table_iter;


typedef struct conn_mgr {
    conn_table table;
    LRUList<uint32_t> cache;


    conn_mgr(size_t cache_size) : table(), cache(cache_size) {}
    ~conn_mgr() {}
} conn_mgr_t;


class PcapLoader {
    private:

        conn_mgr_t connection_manager;
        TcpReassemblyConfiguration cleanup_configuration;
        TcpReassembly reassembler;

        static void on_message_ready_callback(int side, TcpStreamData tcp_data, void* user_cookie);
        static void on_connection_start_callback(ConnectionData connection_data, void* user_cookie);
        static void on_connection_end_callback(ConnectionData connection_data, TcpReassembly::ConnectionEndReason reason, void* user_cookie);


    public:

        // PcapLoader();
        PcapLoader(size_t cache_size=64);
        // Watch();
        bool parse(std::string filename);
        ~PcapLoader();

};