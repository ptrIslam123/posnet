#include "ethernet_builder.h"

#include "src/utils/sock_addr_convertor.h"

#include <stdexcept>
#include <sstream>
#include <cstring>

#include <netinet/in.h>

namespace posnet {
    
EthernetBuilder::EthernetBuilder(const RawFrameVieType rawFrame):
m_frame(reinterpret_cast<HeaderStructType*>(rawFrame.data()))
{}

EthernetBuilder& EthernetBuilder::setDestMacAddress(const std::string_view macAddr) &
{
    const auto result = posnet::utils::StrToMacAddr(macAddr);
    if (result) {
        std::memcpy(m_frame->h_dest, result->data(), result->size());
    } else {
        std::stringstream ss;
        ss << "Could not convert destination str mac-address=" << macAddr << " to binary mac-address. Invalid str mac-address";
        throw std::runtime_error(ss.str());
    }
    return *this;
}

EthernetBuilder& EthernetBuilder::setSourceMacAddress(const std::string_view macAddr) &
{
    const auto result = posnet::utils::StrToMacAddr(macAddr);
    if (result) {
        std::memcpy(m_frame->h_source, result->data(), result->size());
    } else {
        std::stringstream ss;
        ss << "Could not convert source str mac-address=" << macAddr << " to binary mac-address. Invalid str mac-address";
        throw std::runtime_error(ss.str());
    }
    return *this;
}

EthernetBuilder& EthernetBuilder::setProtocol(const ProtocolType protocol) &
{
    switch (protocol) {
        case ProtocolType::IP: {
            m_frame->h_proto = htons(ETH_P_IP);
            break;
        }
        case ProtocolType::ARP: {
            m_frame->h_proto = htons(ETH_P_ARP);
            break;
        }
        case ProtocolType::RARP: {
            m_frame->h_proto = htons(ETH_P_RARP);
            break;
        }
        default:
            throw std::runtime_error("Could not set ethernet protocol type(Undefined EthernetViewer::ProtocolType)");
    }

    return *this;
}

} // namespace posnet
