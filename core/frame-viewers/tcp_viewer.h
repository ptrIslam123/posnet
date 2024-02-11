#ifndef VS_TCP_VIEWER_H
#define VS_TCP_VIEWER_H

#include "ip_viewer.h"

#include <ostream>

#include <netinet/tcp.h>

namespace net::posix::low_level {

/**
 * @brief This class represents of TCP frame.
 * @details Description of TCP layer:
 * source: The source port number.
 * 
 * dest: The destination port number.
 * 
 * seq: The sequence number of the first data octet in this segment (except when SYN is present).
 * 
 * ack_seq: The acknowledgment number of the next sequence number the sender expects to receive (only used with ACK).
 * 
 * doff: The length of the TCP header in 32-bit words. This field is 4 bits long and the minimum value is 5 (indicating a 20-byte header).
 * 
 * res1: Reserved for future use and should be set to zero.
 * 
 * res2: Reserved for future use and should be set to zero.
 * 
 * urg: A flag indicating if the urgent pointer field is in use.
 * 
 * ack: A flag indicating if the acknowledgment field is in use.
 * 
 * psh: A flag indicating if the PUSH function is in use.
 * 
 * rst: A flag indicating if the connection should be reset.
 * 
 * syn: A flag indicating if the synchronize sequence numbers to start a connection.
 * 
 * fin: A flag indicating if this is the last segment from the sender.
 * 
 * window: The number of data octets that the sender of this segment is willing to accept.
 * 
 * check: The checksum of the TCP header, the data, and a pseudo-header.
 * 
 * urg_ptr: This field is only used if the urgent pointer (URG) flag is set.
 */  
class TcpViewer final {
public:
    using RawFrameViewType = IpViewer::RawFrameViewType;
    using ConstRawFrameViewType = IpViewer::ConstRawFrameViewType;
    using HeaderStructType = struct tcphdr;
    using PortType = int;

    explicit TcpViewer(const IpViewer& ipViewer);
    explicit TcpViewer(RawFrameViewType rawFrame);
    explicit TcpViewer(ConstRawFrameViewType rawFrame);

    PortType getSourcePort();
    PortType getDestPort();
    unsigned int getSequenceNumber();
    unsigned int getAcknowledgeNumber();
    unsigned int getHeaderLengthInBytes();
    unsigned int getWindowSize();
    unsigned int getCheckSum();
    bool getUrgentFlag();
    bool getAcknowledgmentFlag();
    bool getPushFlag();
    bool getResetFlag();
    bool getSynchronizeFlag();
    bool getFinishFlag();

    PortType getSourcePort() const;
    PortType getDestPort() const;
    unsigned int getSequenceNumber() const;
    unsigned int getAcknowledgeNumber() const;
    unsigned int getHeaderLengthInBytes() const;
    unsigned int getWindowSize() const;
    unsigned int getCheckSum() const;
    bool getUrgentFlag() const;
    bool getAcknowledgmentFlag() const;
    bool getPushFlag() const;
    bool getResetFlag() const;
    bool getSynchronizeFlag() const;
    bool getFinishFlag() const;

    std::ostream& operator<<(std::ostream& os) const;

private:
    HeaderStructType* m_tcpFrame;
    ConstRawFrameViewType m_rawFrame;
};

std::ostream& operator<<(std::ostream& os, const TcpViewer& tcpViewer);

} //! namespace net::posix::low_level


#endif //! VS_TCP_VIEWER_H