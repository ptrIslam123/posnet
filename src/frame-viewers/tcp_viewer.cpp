#include "tcp_viewer.h"

#include "ethernet_viewer.h"

#include <arpa/inet.h>

namespace {

constexpr unsigned char FlagsSet = 0x00;

} //! namespace

namespace posnet {
    
TcpViewer::TcpViewer(const IpViewer& ipViewer):
m_tcpFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        ipViewer.getAsRawFrame().data() + sizeof(EthernetViewer::HeaderStructType) + ipViewer.getHeaderLengthInBytes()))),
m_rawFrame(ipViewer.getAsRawFrame())
{}

TcpViewer::TcpViewer(RawFrameViewType rawFrame):
m_tcpFrame(reinterpret_cast<HeaderStructType*>(rawFrame.data() + sizeof(EthernetViewer::HeaderStructType) + IpViewer(rawFrame).getHeaderLengthInBytes())),
m_rawFrame(rawFrame)
{}

TcpViewer::TcpViewer(ConstRawFrameViewType rawFrame):
m_tcpFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        rawFrame.data() + sizeof(EthernetViewer::HeaderStructType) + IpViewer(rawFrame).getHeaderLengthInBytes()))),
m_rawFrame(rawFrame)
{}

TcpViewer::PortType TcpViewer::getSourcePort()
{
    return ntohs(m_tcpFrame->source);
}

TcpViewer::PortType TcpViewer::getDestPort()
{
    return ntohs(m_tcpFrame->dest);
}

unsigned int TcpViewer::getSequenceNumber()
{
    return ntohl(m_tcpFrame->seq);
}

unsigned int TcpViewer::getAcknowledgeNumber()
{
    return ntohl(m_tcpFrame->ack_seq);
}

unsigned int TcpViewer::getHeaderLengthInBytes()
{
    return static_cast<int>(m_tcpFrame->doff)*4;
}

unsigned int TcpViewer::getWindowSize()
{
    return ntohs(m_tcpFrame->window);
}

unsigned int TcpViewer::getCheckSum()
{
    return ntohs(m_tcpFrame->check);
}

bool TcpViewer::getUrgentFlag()
{
    return static_cast<bool>(m_tcpFrame->urg & FlagsSet);
}

bool TcpViewer::getAcknowledgmentFlag()
{
    return static_cast<bool>(m_tcpFrame->ack & FlagsSet);
}

bool TcpViewer::getPushFlag()
{
    return static_cast<bool>(m_tcpFrame->psh & FlagsSet);
}

bool TcpViewer::getResetFlag()
{
    return static_cast<bool>(m_tcpFrame->rst & FlagsSet);
}

bool TcpViewer::getSynchronizeFlag()
{
    return static_cast<bool>(m_tcpFrame->syn & FlagsSet);
}

bool TcpViewer::getFinishFlag()
{
    return static_cast<bool>(m_tcpFrame->fin & FlagsSet);
}

TcpViewer::PortType TcpViewer::getSourcePort() const
{
    return ntohs(m_tcpFrame->source);
}

TcpViewer::PortType TcpViewer::getDestPort() const
{
    return ntohs(m_tcpFrame->dest);
}

unsigned int TcpViewer::getSequenceNumber() const
{
    return ntohl(m_tcpFrame->seq);
}

unsigned int TcpViewer::getAcknowledgeNumber() const
{
    return ntohl(m_tcpFrame->ack_seq);
}

unsigned int TcpViewer::getHeaderLengthInBytes() const
{
    return static_cast<int>(m_tcpFrame->doff)*4;
}

unsigned int TcpViewer::getWindowSize() const
{
    return ntohs(m_tcpFrame->window);
}

unsigned int TcpViewer::getCheckSum() const
{
    return ntohs(m_tcpFrame->check);
}

bool TcpViewer::getUrgentFlag() const
{
    return static_cast<bool>(m_tcpFrame->urg & FlagsSet);
}

bool TcpViewer::getAcknowledgmentFlag() const
{
    return static_cast<bool>(m_tcpFrame->ack & FlagsSet);
}

bool TcpViewer::getPushFlag() const
{
    return static_cast<bool>(m_tcpFrame->psh & FlagsSet);
}

bool TcpViewer::getResetFlag() const
{
    return static_cast<bool>(m_tcpFrame->rst & FlagsSet);
}

bool TcpViewer::getSynchronizeFlag() const
{
    return static_cast<bool>(m_tcpFrame->syn & FlagsSet);
}

bool TcpViewer::getFinishFlag() const
{
    return static_cast<bool>(m_tcpFrame->fin & FlagsSet);
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