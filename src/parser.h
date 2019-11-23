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
    public:
        enum Side : int {Client = 0, Server, Unknown};

    private:
        struct reassembly_state_t {
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
            pcpp::LRUList<uint32_t> cache;


            conn_mgr_t(size_t cache_size) : table(), cache(cache_size) {}
        };


        bool autoremove;

        conn_mgr_t connection_manager;
        pcpp::TcpReassemblyConfiguration cleanup_configuration;
        pcpp::TcpReassembly reassembler;

        static void on_message_ready_callback(int side, pcpp::TcpStreamData tcp_data, void* user_cookie);
        static void on_connection_start_callback(pcpp::ConnectionData connection_data, void* user_cookie);
        static void on_connection_end_callback(pcpp::ConnectionData connection_data, pcpp::TcpReassembly::ConnectionEndReason reason, void* user_cookie);


    public:

        Logger _logger;

        PcapLoader(size_t cache_size=64);
        // Watch();
        bool parse(std::string filename);
        ~PcapLoader();

        inline bool get_autoremove() { return autoremove; };
        inline void set_autoremove(bool state) { autoremove = state; };

};
