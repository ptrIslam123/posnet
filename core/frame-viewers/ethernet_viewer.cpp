#include "ethernet_viewer.h"

#include "utils/sock_addr_convertor.h"

#include <arpa/inet.h>

namespace {

std::string_view ProtocolToStr(const net::posix::low_level::EthernetViewer::ProtocolType protocol)
{
    using ProtocolType = net::posix::low_level::EthernetViewer::ProtocolType;
    switch (protocol) {
        case ProtocolType::ARP: return "ARP";
        case ProtocolType::IP: return "IP";
        case ProtocolType::RARP: return "RARP";
        default:
            return "Undefined";
    }
}

} //! namespace

namespace net::posix::low_level {

EthernetViewer::EthernetViewer(const RawFrameViewType rawFrame):
m_ethernetFrame(reinterpret_cast<HeaderStructType*>(rawFrame.data())),
m_rawFrame(rawFrame)
{}

std::string EthernetViewer::getDestMacAddressAsStr()
{
    return utils::net::posix::low_level::MacAddrToStr(m_ethernetFrame->h_dest);
}

std::string EthernetViewer::getSourceMacAddressAsStr()
{
    return utils::net::posix::low_level::MacAddrToStr(m_ethernetFrame->h_source);
}

EthernetViewer::ProtocolType EthernetViewer::getProtocol()
{
    switch (ntohs(m_ethernetFrame->h_proto)) {
        case 0x0800: return ProtocolType::IP;
        case 0x0806: return ProtocolType::ARP;
        case 0x8035: return ProtocolType::RARP;
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
    return utils::net::posix::low_level::MacAddrToStr(m_ethernetFrame->h_dest);
}

std::string EthernetViewer::getSourceMacAddressAsStr() const
{
    return utils::net::posix::low_level::MacAddrToStr(m_ethernetFrame->h_source);
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

} //! namespace net::posix::low_level