#ifndef VS_UPD_VIEWER_H
#define VS_UPD_VIEWER_H

#include "ip_viewer.h"

#include <ostream>

#include <netinet/udp.h>

namespace net::posix::low_level {

/**
 * @brief This class represents of UDP frame.
 * @details Description of UDP layer.
 * source: This is the source port number. It is used for multiplexing and demultiplexing of UDP datagrams.
 * 
 * dest: This is the destination port number. It is used for multiplexing and demultiplexing of UDP datagrams.
 * 
 * len: This is the length of the UDP header and data in bytes. It does not include the length of the IP header.
 * 
 * check: This is the checksum of the UDP header and data. It is used for error-checking of the UDP header and data.
 */
class UdpViewer final {
public:
    using RawFrameViewType = IpViewer::RawFrameViewType;
    using ConstRawFrameViewType = IpViewer::ConstRawFrameViewType;
    using HeaderStructType = struct udphdr;
    using PortType = int;

    explicit UdpViewer(const IpViewer& ipViewer);
    explicit UdpViewer(RawFrameViewType rawFrame);
    explicit UdpViewer(ConstRawFrameViewType rawFrame);

    PortType getSourcePort();
    PortType getDestPort();
    int getUdpDataGramLength();
    int getCheckSum();

    PortType getSourcePort() const;
    PortType getDestPort() const;
    int getUdpDataGramLength() const;
    int getCheckSum() const;

    std::ostream& operator<<(std::ostream& os) const;

private:
    HeaderStructType* m_udpFrame;
    ConstRawFrameViewType m_rawFrame;
};
    
std::ostream& operator<<(std::ostream& os, const UdpViewer& updViewer);

} //! namespace net::posix::low_level


#endif //! VS_UPD_VIEWER_H