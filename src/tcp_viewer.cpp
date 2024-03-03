#include "frame-viewers/tcp_viewer.h"

#include "frame-viewers/ethernet_viewer.h"

#include <arpa/inet.h>

namespace {

constexpr unsigned char FlagsSet = 0x00;

} //! namespace

namespace posnet {
    
TcpViewer::TcpViewer(IpViewer ipViewer):
m_frame(reinterpret_cast<HeaderStructType*>(
    ipViewer.getFrameHeaderStart() + ipViewer.getHeaderLengthInBytes()))
{}

TcpViewer::TcpViewer(RawFrameViewType rawFrame):
m_frame(nullptr)
{
    const auto ethernetHeaderLength = EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;
    const auto ipHeaderLength = IpViewer(RawFrameViewType{
        rawFrame.data() + ethernetHeaderLength, 
        rawFrame.size() - ethernetHeaderLength 
    }).getHeaderLengthInBytes();

    m_frame = reinterpret_cast<HeaderStructType*>(rawFrame.data() + ethernetHeaderLength + ipHeaderLength);
}

TcpViewer::TcpViewer(ConstRawFrameViewType rawFrame):
m_frame(nullptr)
{
    const auto ethernetHeaderLength = EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;
    const auto ipHeaderLength = IpViewer(ConstRawFrameViewType{
        rawFrame.data() + ethernetHeaderLength, 
        rawFrame.size() - ethernetHeaderLength 
    }).getHeaderLengthInBytes();
    
    m_frame = reinterpret_cast<HeaderStructType*>(const_cast<RawFrameViewType::value_type*>(rawFrame.data() + ethernetHeaderLength + ipHeaderLength));
}

TcpViewer::PortType TcpViewer::getSourcePort()
{
    return ntohs(m_frame->source);
}

TcpViewer::PortType TcpViewer::getDestPort()
{
    return ntohs(m_frame->dest);
}

unsigned int TcpViewer::getSequenceNumber()
{
    return ntohl(m_frame->seq);
}

unsigned int TcpViewer::getAcknowledgeNumber()
{
    return ntohl(m_frame->ack_seq);
}

unsigned int TcpViewer::getHeaderLengthInBytes()
{
    return static_cast<int>(m_frame->doff)*4;
}

unsigned int TcpViewer::getWindowSize()
{
    return ntohs(m_frame->window);
}

unsigned int TcpViewer::getCheckSum()
{
    return ntohs(m_frame->check);
}

bool TcpViewer::getUrgentFlag()
{
    return static_cast<bool>(m_frame->urg & FlagsSet);
}

bool TcpViewer::getAcknowledgmentFlag()
{
    return static_cast<bool>(m_frame->ack & FlagsSet);
}

bool TcpViewer::getPushFlag()
{
    return static_cast<bool>(m_frame->psh & FlagsSet);
}

bool TcpViewer::getResetFlag()
{
    return static_cast<bool>(m_frame->rst & FlagsSet);
}

bool TcpViewer::getSynchronizeFlag()
{
    return static_cast<bool>(m_frame->syn & FlagsSet);
}

bool TcpViewer::getFinishFlag()
{
    return static_cast<bool>(m_frame->fin & FlagsSet);
}

TcpViewer::PortType TcpViewer::getSourcePort() const
{
    return ntohs(m_frame->source);
}

TcpViewer::PortType TcpViewer::getDestPort() const
{
    return ntohs(m_frame->dest);
}

unsigned int TcpViewer::getSequenceNumber() const
{
    return ntohl(m_frame->seq);
}

unsigned int TcpViewer::getAcknowledgeNumber() const
{
    return ntohl(m_frame->ack_seq);
}

unsigned int TcpViewer::getHeaderLengthInBytes() const
{
    return static_cast<int>(m_frame->doff)*4;
}

unsigned int TcpViewer::getWindowSize() const
{
    return ntohs(m_frame->window);
}

unsigned int TcpViewer::getCheckSum() const
{
    return ntohs(m_frame->check);
}

bool TcpViewer::getUrgentFlag() const
{
    return static_cast<bool>(m_frame->urg & FlagsSet);
}

bool TcpViewer::getAcknowledgmentFlag() const
{
    return static_cast<bool>(m_frame->ack & FlagsSet);
}

bool TcpViewer::getPushFlag() const
{
    return static_cast<bool>(m_frame->psh & FlagsSet);
}

bool TcpViewer::getResetFlag() const
{
    return static_cast<bool>(m_frame->rst & FlagsSet);
}

bool TcpViewer::getSynchronizeFlag() const
{
    return static_cast<bool>(m_frame->syn & FlagsSet);
}

bool TcpViewer::getFinishFlag() const
{
    return static_cast<bool>(m_frame->fin & FlagsSet);
}

std::uint8_t* TcpViewer::getFrameHeaderStart()
{
    return reinterpret_cast<std::uint8_t*>(m_frame);
}

std::ostream& TcpViewer::operator<<(std::ostream& os) const
{
    os << "TCP header {\n";
    os << "\tsource-port=" << getSourcePort() << "\n";
    os << "\tdestination-port=" << getDestPort() << "\n";
    os << "\tsequence number=" << getSequenceNumber() << "\n";
    os << "\tacknowledge number=" << getAcknowledgeNumber() << "\n";
    os << "\theader length in bytes=" << getHeaderLengthInBytes() << "\n";
    os << "\twindow size=" << getWindowSize() << "\n";
    os << "\tcheck-sum=" << getCheckSum() << "\n";
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TcpViewer& tcpViewer)
{
    return tcpViewer.operator<<(os);
} 

} //! namespace posnet