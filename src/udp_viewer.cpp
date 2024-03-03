#include "frame-viewers/udp_viewer.h"

#include "frame-viewers/ethernet_viewer.h"

#include <arpa/inet.h>

namespace posnet {
    
UdpViewer::UdpViewer(IpViewer ipViewer):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(ipViewer.getStart()), ipViewer.getSize()),
m_frame(reinterpret_cast<HeaderStructType*>(
    ipViewer.getFrameHeaderStart() + ipViewer.getHeaderLengthInBytes()))
{}

UdpViewer::UdpViewer(const RawFrameViewType rawFrame):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(rawFrame.data()), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame(reinterpret_cast<HeaderStructType*>(rawFrame.data()))
{}

UdpViewer::UdpViewer(const ConstRawFrameViewType rawFrame):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(rawFrame.data()), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(rawFrame.data())))
{}

UdpViewer::PortType UdpViewer::getSourcePort()
{
    return ntohs(m_frame->source);
}

UdpViewer::PortType UdpViewer::getDestPort()
{
    return ntohs(m_frame->dest);
}

unsigned int UdpViewer::getUdpDataGramLength()
{
    return ntohs(m_frame->len);
}

unsigned int UdpViewer::getCheckSum()
{
    return ntohs(m_frame->check);
}

UdpViewer::PortType UdpViewer::getSourcePort() const
{
    return ntohs(m_frame->source);
}

UdpViewer::PortType UdpViewer::getDestPort() const
{
    return ntohs(m_frame->dest);
}

unsigned int UdpViewer::getUdpDataGramLength() const
{
    return ntohs(m_frame->len);
}

unsigned int UdpViewer::getCheckSum() const
{
    return ntohs(m_frame->check);
}

std::uint8_t* UdpViewer::getFrameHeaderStart()
{
    return reinterpret_cast<std::uint8_t*>(m_frame);
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
