#pragma once

#include <memory>
#include <vector>
#include <string>

// Description: representation of one conversation.
// After object creation, you can only add new data. All data is shared between
// copies of this object, so it can be copied and moved freely.
// Sharing between threads is not safe.

class StreamData
{
    public:
        enum Side : int {Client = 0, Server, Unknown};

    private:
        // each new entry is a side switch
        std::shared_ptr< std::vector<std::string> > m_data;

    public:
              Side     start_side;
        const uint16_t src_port;
        const uint16_t dst_port;

        inline StreamData(uint16_t a_src_port, uint16_t a_dst_port)
            : StreamData(Side::Unknown, a_src_port, a_dst_port)
        {}
        inline StreamData(Side a_start_side, uint16_t a_src_port, uint16_t a_dst_port)
            : m_data     (new std::vector<std::string>)
            , start_side (a_start_side)
            , src_port   (a_src_port)
            , dst_port   (a_dst_port)
        {}
        StreamData()                  = delete;
        StreamData(const StreamData&) = default;
        StreamData(StreamData&&)      = default;

        void push_back(const std::string&);
        void push_back(std::string&&);

        inline const std::vector<std::string>& data() const
        {
            return *m_data;
        }

        inline static Side flip_side(Side side)
        {
            switch (side) {
                case Side::Client:
                    return Side::Server;
                case Side::Server:
                    return Side::Client;
                default:
                    return Side::Unknown;
            }
        }
};


namespace std
{
    inline std::string to_string(StreamData::Side side)
    {
        switch (side) {
            case StreamData::Side::Unknown:
                return "Unknown";
            case StreamData::Side::Client:
                return "Client";
            case StreamData::Side::Server:
                return "Server";
        }
        return "Undefined";
    }
}
