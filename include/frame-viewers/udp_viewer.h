#ifndef VS_UPD_VIEWER_H
#define VS_UPD_VIEWER_H

#include "include/base_frame.h"
#include "include/frame-viewers/ip_viewer.h"

#include <ostream>

#include <netinet/udp.h>

namespace posnet {

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
class UdpViewer final : public BaseFrame {
public:
    static constexpr auto DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = sizeof(struct udphdr);

    using RawFrameViewType = IpViewer::RawFrameViewType;
    using ConstRawFrameViewType = IpViewer::ConstRawFrameViewType;
    using HeaderStructType = struct udphdr;
    using PortType = int;

    explicit UdpViewer(IpViewer ipViewer);
    explicit UdpViewer(RawFrameViewType rawFrame);
    explicit UdpViewer(ConstRawFrameViewType rawFrame);

    PortType getSourcePort();
    PortType getDestPort();
    unsigned int getUdpDataGramLength();
    unsigned int getCheckSum();

    PortType getSourcePort() const;
    PortType getDestPort() const;
    unsigned int getUdpDataGramLength() const;
    unsigned int getCheckSum() const;

    std::uint8_t* getFrameHeaderStart();

    std::ostream& operator<<(std::ostream& os) const;

private:
    HeaderStructType* m_frame;
};
    
std::ostream& operator<<(std::ostream& os, const UdpViewer& updViewer);

} //! namespace posnet


#endif //! VS_UPD_VIEWER_H
