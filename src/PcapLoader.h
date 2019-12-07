#pragma once

#include <cinttypes>
#include <map>

#include <LRUList.h>
#include <TcpReassembly.h>

#include "StreamData.h"
#include "logger.h"


class PcapLoader
{
    private:
        struct reassembly_state_t
        {
            StreamData::Side current_side;
            uint64_t         msg_count;
            std::string      buffer;
            StreamData       all_data;

            reassembly_state_t() = delete;
            inline reassembly_state_t(uint16_t s, uint16_t d)
                : reassembly_state_t(StreamData::Side::Unknown, s, d) {}
            inline reassembly_state_t( StreamData::Side start_side
                                     , uint16_t arg_src_port
                                     , uint16_t arg_dest_port
                                     )
                : current_side (start_side)
                , msg_count    (0)
                , buffer       ()
                , all_data     (start_side, arg_src_port, arg_dest_port)
                {}
        };

        struct conn_mgr_t
        {
            std::map<uint32_t, reassembly_state_t> table;
            pcpp::LRUList<uint32_t> cache;


            conn_mgr_t(size_t cache_size) : table(), cache(cache_size) {}
        };

        static logger::Logger logger;


        bool autoremove;

        conn_mgr_t connection_manager;
        pcpp::TcpReassemblyConfiguration cleanup_configuration;
        pcpp::TcpReassembly reassembler;

        static void on_message_ready_callback( int side
                                             , pcpp::TcpStreamData tcp_data
                                             , void* user_cookie
                                             );
        static void on_connection_start_callback(
                  pcpp::ConnectionData connection_data
                , void* user_cookie
                );
        static void on_connection_end_callback(
                  pcpp::ConnectionData connection_data
                , pcpp::TcpReassembly::ConnectionEndReason reason
                , void* user_cookie
                );


    public:
        PcapLoader(size_t cache_size=64);
        // Watch();
        bool parse(const std::string& filename);
        ~PcapLoader();

        inline bool get_autoremove() { return autoremove; };
        inline void set_autoremove(bool state) { autoremove = state; };
};
