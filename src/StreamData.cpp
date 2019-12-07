#include "StreamData.h"


void StreamData::push_back(const std::string& data)
{
    m_data->push_back(data);
}


void StreamData::push_back(std::string&& data)
{
    m_data->push_back(data);
}
