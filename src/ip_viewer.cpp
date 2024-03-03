#include "frame-viewers/ip_viewer.h"

#include "utils/sock_addr_convertor.h"

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

IpViewer::IpViewer(EthernetViewer ethernetViewer):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(ethernetViewer.getStart()), ethernetViewer.getSize()),
m_frame(reinterpret_cast<HeaderStructType*>(
    ethernetViewer.getFrameHeaderStart() + EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES))
{}

IpViewer::IpViewer(const RawFrameViewType rawFrame):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(rawFrame.data()), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame(reinterpret_cast<HeaderStructType*>(rawFrame.data()))
{}

IpViewer::IpViewer(const ConstRawFrameViewType rawFrame):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(rawFrame.data()), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(rawFrame.data())))
{}

IpViewer::VersionType IpViewer::getVersion()
{
    return (m_frame->version == 4 ? VersionType::V4 : VersionType::V6);
}

IpViewer::ProtocolType IpViewer::getProtocol()
{
    switch (m_frame->protocol) {
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
    return m_frame->tos;
}

unsigned int IpViewer::getHeaderLengthInBytes()
{
    return static_cast<unsigned int>(m_frame->ihl) * 4;
}

unsigned int IpViewer::getTotalLength()
{
    return ntohs(m_frame->tot_len);
}

unsigned int IpViewer::getId()
{
    return ntohs(m_frame->id);
}

unsigned int IpViewer::getFragmentOffset()
{
    return (m_frame->frag_off);
}

unsigned int IpViewer::getTTL()
{
    return (m_frame->ttl);
}

unsigned int IpViewer::getCheckSum()
{
    return ntohs(m_frame->check);
}

std::string IpViewer::getSourceIpAddressAsStr()
{
    return posnet::utils::IpAddrToStr(m_frame->saddr);
}

std::string IpViewer::getDestIpAddressAsStr()
{
    return posnet::utils::IpAddrToStr(m_frame->daddr);
}

IpViewer::VersionType IpViewer::getVersion() const
{
    return (m_frame->version == 4 ? VersionType::V4 : VersionType::V6);
}

IpViewer::ProtocolType IpViewer::getProtocol() const
{
    switch (m_frame->protocol) {
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
    return m_frame->tos;
}

unsigned int IpViewer::getHeaderLengthInBytes() const
{
    return static_cast<unsigned int>(m_frame->ihl) * 4;
}

unsigned int IpViewer::getTotalLength() const
{
    return ntohs(m_frame->tot_len);
}

unsigned int IpViewer::getId() const
{
    return ntohs(m_frame->id);
}

unsigned int IpViewer::getFragmentOffset() const
{
    return (m_frame->frag_off);
}

unsigned int IpViewer::getTTL() const
{
    return (m_frame->ttl);
}

unsigned int IpViewer::getCheckSum() const
{
    return ntohs(m_frame->check);
}

std::string IpViewer::getSourceIpAddressAsStr() const
{
    return posnet::utils::IpAddrToStr(m_frame->saddr);
}

std::string IpViewer::getDestIpAddressAsStr() const
{
    return posnet::utils::IpAddrToStr(m_frame->daddr);
}

std::uint8_t* IpViewer::getFrameHeaderStart()
{
    return reinterpret_cast<std::uint8_t*>(m_frame);
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

} //! namespace posnet