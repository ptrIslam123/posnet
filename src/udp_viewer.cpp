#include "frame-viewers/udp_viewer.h"

#include "frame-viewers/ethernet_viewer.h"

#include <arpa/inet.h>

namespace posnet {
    
UdpViewer::UdpViewer(const IpViewer& ipViewer):
m_udpFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        ipViewer.getAsRawFrame().data() + sizeof(EthernetViewer::HeaderStructType) + ipViewer.getHeaderLengthInBytes()))),
m_rawFrame(ipViewer.getAsRawFrame())
{}

UdpViewer::UdpViewer(const RawFrameViewType rawFrame):
m_udpFrame(reinterpret_cast<HeaderStructType*>(
       rawFrame.data() + sizeof(EthernetViewer::HeaderStructType) + IpViewer(rawFrame).getHeaderLengthInBytes())),
m_rawFrame(rawFrame)
{}

UdpViewer::UdpViewer(const ConstRawFrameViewType rawFrame):
m_udpFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        rawFrame.data() + sizeof(EthernetViewer::HeaderStructType) + IpViewer(rawFrame).getHeaderLengthInBytes()))),
m_rawFrame(rawFrame)
{}

UdpViewer::PortType UdpViewer::getSourcePort()
{
    return ntohs(m_udpFrame->source);
}

UdpViewer::PortType UdpViewer::getDestPort()
{
    return ntohs(m_udpFrame->dest);
}

unsigned int UdpViewer::getUdpDataGramLength()
{
    return ntohs(m_udpFrame->len);
}

unsigned int UdpViewer::getCheckSum()
{
    return ntohs(m_udpFrame->check);
}

UdpViewer::PortType UdpViewer::getSourcePort() const
{
    return ntohs(m_udpFrame->source);
}

UdpViewer::PortType UdpViewer::getDestPort() const
{
    return ntohs(m_udpFrame->dest);
}

unsigned int UdpViewer::getUdpDataGramLength() const
{
    return ntohs(m_udpFrame->len);
}

unsigned int UdpViewer::getCheckSum() const
{
    return ntohs(m_udpFrame->check);
}

std::ostream& UdpViewer::operator<<(std::ostream& os) const
{
    os << "UDP header {\n";
    os << "\tsource-port=" << getSourcePort() << "\n";
    os << "\tdestination-port=" << getDestPort() << "\n";
    os << "\tudp-datagram-length=" << getUdpDataGramLength() << "\n";
    os << "\tchecksum=" << getCheckSum() << "\n";
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const UdpViewer& updViewer)
{
    return updViewer.operator<<(os);
}

} //! namespace posnet
