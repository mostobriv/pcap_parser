#pragma once

#include <cinttypes>
#include <map>

#include <LRUList.h>
#include <TcpReassembly.h>

#include "StreamData.h"
#include "logger.h"
#include "ThreadQueue.hpp"


class PcapLoader
{
    private:
        struct reassembly_state_t
        {
            StreamData::Side current_side;
            uint64_t         msg_count;
            std::string      buffer; // one side data
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
            ThreadQueue<StreamData>& queue;

            conn_mgr_t(size_t cache_size, ThreadQueue<StreamData>& q)
                : table(), cache(cache_size), queue(q) {}
        };

        static logger::Logger logger;

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
        PcapLoader(ThreadQueue<StreamData>&, size_t cache_size=64);
        void parse(const std::string& filename);
        void parse_many(const std::vector<std::string>&);
        ~PcapLoader();
};
