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


class PcapLoader {
    using namespace pcpp;
    private:
        struct reassembly_state_t {
            enum struct Side : int {Client = 0, Server, Unknown};

            Side current_side;
            uint32_t msg_count;
            uint16_t src_port;
            uint16_t dest_port;


            inline reassembly_state_t() : reassembly_state_t(Side::Unknown, 0, 0, 0) {};
            inline reassembly_state_t(uint16_t s, uint16_t d)
                : reassembly_state_t(Side::Unknown, 0, s, d) {};
            inline reassembly_state_t(Side start_side, uint32_t arg_msg_count,
                                      uint16_t arg_src_port, uint16_t arg_dest_port)
                : current_side (start_side)
                , msg_count    (arg_msg_count)
                , src_port     (arg_src_port)
                , dest_port    (arg_dest_port)
                {}
        };

        struct conn_mgr_t {
            std::map<uint32_t, reassembly_state_t> table;
            LRUList<uint32_t> cache;


            conn_mgr_t(size_t cache_size) : table(), cache(cache_size) {}
        };


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

        inline bool get_autoremove() { return autoremove; };
        inline void set_autoremove(bool state) { autoremove = state; };

};
