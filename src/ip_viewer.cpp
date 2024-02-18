#include "frame-viewers/ip_viewer.h"

#include "utils/sock_addr_convertor.h"

#include <unordered_map>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace {

std::string_view ProtocolToStr(const posnet::IpViewer::ProtocolType protocol)
{
    using ProtocolType = posnet::IpViewer::ProtocolType;
    switch (protocol) {
        case ProtocolType::TCP: return "TCP";
        case ProtocolType::UDP: return "UDP";
        case ProtocolType::ICMP: return "ICMP";
        default: 
            return "Undefined";
    }
}

} //! namespace

namespace posnet {

IpViewer::IpViewer(const EthernetViewer& ethernetViewer):
m_ipFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        ethernetViewer.getAsRawFrame().data() + sizeof(EthernetViewer::HeaderStructType)))),
m_rawFrame(ethernetViewer.getAsRawFrame())
{}

IpViewer::IpViewer(const RawFrameViewType rawFrame):
m_ipFrame(reinterpret_cast<HeaderStructType*>(rawFrame.data() + sizeof(EthernetViewer::HeaderStructType))),
m_rawFrame(rawFrame)
{}

IpViewer::IpViewer(const ConstRawFrameViewType rawFrame):
m_ipFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        rawFrame.data() + sizeof(EthernetViewer::HeaderStructType)))),
m_rawFrame(rawFrame)
{}

IpViewer::VersionType IpViewer::getVersion()
{
    return (m_ipFrame->version == 4 ? VersionType::V4 : VersionType::V6);
}

IpViewer::ProtocolType IpViewer::getProtocol()
{
    switch (m_ipFrame->protocol) {
        case IPPROTO_ICMP: return ProtocolType::ICMP;
        case IPPROTO_TCP: return ProtocolType::TCP;
        case IPPROTO_UDP: return ProtocolType::UDP;
        default:
            return ProtocolType::Undefined;
    }
}

std::string_view IpViewer::getProtocolAsStr()
{
   return ProtocolToStr(getProtocol());
}

unsigned int IpViewer::getTypeOfService()
{
    return m_ipFrame->tos;
}

unsigned int IpViewer::getHeaderLengthInBytes()
{
    return static_cast<unsigned int>(m_ipFrame->ihl) * 4;
}

unsigned int IpViewer::getTotalLength()
{
    return ntohs(m_ipFrame->tot_len);
}

unsigned int IpViewer::getId()
{
    return ntohs(m_ipFrame->id);
}

unsigned int IpViewer::getFragmentOffset()
{
    return (m_ipFrame->frag_off);
}

unsigned int IpViewer::getTTL()
{
    return (m_ipFrame->ttl);
}

unsigned int IpViewer::getCheckSum()
{
    return ntohs(m_ipFrame->check);
}

std::string IpViewer::getSourceIpAddressAsStr()
{
    return posnet::utils::IpAddrToStr(m_ipFrame->saddr);
}

std::string IpViewer::getDestIpAddressAsStr()
{
    return posnet::utils::IpAddrToStr(m_ipFrame->daddr);
}

IpViewer::VersionType IpViewer::getVersion() const
{
    return (m_ipFrame->version == 4 ? VersionType::V4 : VersionType::V6);
}

IpViewer::ProtocolType IpViewer::getProtocol() const
{
    switch (m_ipFrame->protocol) {
        case IPPROTO_ICMP: return ProtocolType::ICMP;
        case IPPROTO_TCP: return ProtocolType::TCP;
        case IPPROTO_UDP: return ProtocolType::UDP;
        default:
            return ProtocolType::Undefined;
    }
}

std::string_view IpViewer::getProtocolAsStr() const
{
    return ProtocolToStr(getProtocol());
}

unsigned int IpViewer::getTypeOfService() const
{
    return m_ipFrame->tos;
}

unsigned int IpViewer::getHeaderLengthInBytes() const
{
    return static_cast<unsigned int>(m_ipFrame->ihl) * 4;
}

unsigned int IpViewer::getTotalLength() const
{
    return ntohs(m_ipFrame->tot_len);
}

unsigned int IpViewer::getId() const
{
    return ntohs(m_ipFrame->id);
}

unsigned int IpViewer::getFragmentOffset() const
{
    return (m_ipFrame->frag_off);
}

unsigned int IpViewer::getTTL() const
{
    return (m_ipFrame->ttl);
}

unsigned int IpViewer::getCheckSum() const
{
    return ntohs(m_ipFrame->check);
}

std::string IpViewer::getSourceIpAddressAsStr() const
{
    return posnet::utils::IpAddrToStr(m_ipFrame->saddr);
}

std::string IpViewer::getDestIpAddressAsStr() const
{
    return posnet::utils::IpAddrToStr(m_ipFrame->daddr);
}

std::ostream& IpViewer::operator<<(std::ostream& os) const
{
    os << "IP header {\n";
    os << "\tversion=";
    switch (getVersion()) {
        case VersionType::V4: {
            os << "IPv4";
            break;
        }
        case VersionType::V6: {
            os << "IPv6";
            break;
        }
    }
    os << "\n";

    os << "\tprotocol=" << getProtocolAsStr() << "\n";
    os << "\tid=" << getId() << "\n";
    os << "\tfragment_offset=" << getFragmentOffset() << "\n";
    os << "\tsource-ip-address=" << getSourceIpAddressAsStr() << "\n";
    os << "\tdestination-ip-address=" << getDestIpAddressAsStr() << "\n";
    os << "\tttl=" << getTTL() << "\n";
    os << "\tcheck_sum=" << getCheckSum() << "\n";
    os << "\ttype_of_service(tos)=" << getTypeOfService() << "\n";
    os << "\ttotal_length=" << getTotalLength() << "\n";
    os << "\theader_length(in bytes)=" << getHeaderLengthInBytes() << "\n";
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const IpViewer& ipViewer)
{
    return ipViewer.operator<<(os);
}

IpViewer::ConstRawFrameViewType IpViewer::getAsRawFrame()
{
    return m_rawFrame;
}

IpViewer::ConstRawFrameViewType IpViewer::getAsRawFrame() const
{
    return m_rawFrame;
}

} //! namespace posnet