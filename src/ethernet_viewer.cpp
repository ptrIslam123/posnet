#include "frame-viewers/ethernet_viewer.h"

#include "utils/sock_addr_convertor.h"

#include <arpa/inet.h>

namespace {

std::string_view ProtocolToStr(const posnet::EthernetViewer::ProtocolType protocol)
{
    using ProtocolType = posnet::EthernetViewer::ProtocolType;
    switch (protocol) {
        case ProtocolType::ARP: return "ARP";
        case ProtocolType::IP: return "IP";
        case ProtocolType::RARP: return "RARP";
        default:
            return "Undefined";
    }
}

} //! namespace

namespace posnet {

EthernetViewer::EthernetViewer(const ConstRawFrameViewType rawFrame):
m_ethernetFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(rawFrame.data()))),
m_rawFrame(rawFrame)
{}

std::string EthernetViewer::getDestMacAddressAsStr()
{
    return posnet::utils::MacAddrToStr(m_ethernetFrame->h_dest);
}

std::string EthernetViewer::getSourceMacAddressAsStr()
{
    return posnet::utils::MacAddrToStr(m_ethernetFrame->h_source);
}

EthernetViewer::ProtocolType EthernetViewer::getProtocol()
{
    switch (ntohs(m_ethernetFrame->h_proto)) {
        using ProtocolType = EthernetViewer::ProtocolType;
        case ETH_P_IP: return ProtocolType::IP;
        case ETH_P_ARP: return ProtocolType::ARP;
        case ETH_P_RARP: return ProtocolType::RARP;
        default:
            return ProtocolType::Undefined;
    }
}

std::string_view EthernetViewer::getProtocolAsStr()
{
    return ProtocolToStr(getProtocol());
}

std::string EthernetViewer::getDestMacAddressAsStr() const
{
    return posnet::utils::MacAddrToStr(m_ethernetFrame->h_dest);
}

std::string EthernetViewer::getSourceMacAddressAsStr() const
{
    return posnet::utils::MacAddrToStr(m_ethernetFrame->h_source);
}

EthernetViewer::ProtocolType EthernetViewer::getProtocol() const
{
    switch (ntohs(m_ethernetFrame->h_proto)) {
        case 0x0800: return ProtocolType::IP;
        case 0x0806: return ProtocolType::ARP;
        case 0x8035: return ProtocolType::RARP;
        default:
            return ProtocolType::Undefined;
    }
}

std::string_view EthernetViewer::getProtocolAsStr() const
{
    return ProtocolToStr(getProtocol());
}

std::ostream& EthernetViewer::operator<<(std::ostream& os) const
{
    os << "Ethernet header {\n";
    os << "\tdestination-mac-address=" << getDestMacAddressAsStr() << "\n";
    os << "\tsource-mac-address=" << getSourceMacAddressAsStr() << "\n";
    os << "\tprotocol=";
    switch (getProtocol()) {
        using ProtocolType = EthernetViewer::ProtocolType; 
        case ProtocolType::IP: {
            os << "IP";
            break;
        }
        case ProtocolType::ARP: {
            os << "ARP";
            break;
        }
        case ProtocolType::RARP: {
            os << "RARP";
            break;
        }
        default:
            os << "Undefined(" << m_ethernetFrame->h_proto << ")";
            break;
    }
    os << "\n";
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const EthernetViewer& ethernetViewer)
{
    return ethernetViewer.operator<<(os);
}

EthernetViewer::ConstRawFrameViewType EthernetViewer::getAsRawFrame()
{
    return m_rawFrame;
}

EthernetViewer::ConstRawFrameViewType EthernetViewer::getAsRawFrame() const
{
    return m_rawFrame;
}

} //! namespace posnet