#ifndef VS_IP_VIEWER_H
#define VS_IP_VIEWER_H

#include "ethernet_viewer.h"

#include <string>
#include <string_view>
#include <ostream>

#include <netinet/ip.h>

namespace net::posix::low_level {

/**
 * @brief This class represents of IP frame.
 * @details Description of IP layer:
 * version: This is a 4-bit field that specifies the IP version. It is usually 4 for IPv4.
 * 
 * ihl: This is a 4-bit field that specifies the length of the header in 32-bit words. 
 * The minimum value is 5 for the basic header without options.
 * 
 * tos: This is an 8-bit field that specifies the type of service. 
 * It is used to provide different types of service classes.
 * 
 * tot_len: This is a 16-bit field that specifies the total length of the datagram, including the header and data.
 * 
 * id: This is a 16-bit field that identifies the datagram. 
 * It is used for fragmentation and reassembly.
 * 
 * frag_off: This is a 16-bit field that specifies the fragment offset for fragmentation. 
 * The first 3 bits are flags (reserved, DF, MF), and the remaining 13 bits are the fragment offset.
 * 
 * ttl: This is an 8-bit field that specifies the time to live for the datagram.
 * 
 * protocol: This is an 8-bit field that specifies the protocol of the data in the datagram. 
 * For example, 6 for TCP, 17 for UDP.
 * 
 * check: This is a 16-bit field that contains the checksum of the header.
 * 
 * saddr: This is a 32-bit field that specifies the source IP address.
 * 
 * daddr: This is a 32-bit field that specifies the destination IP address.
 */
class IpViewer final {
public:
    using RawFrameViewType = EthernetViewer::RawFrameViewType;
    using ConstRawFrameViewType = EthernetViewer::ConstRawFrameViewType;
    using HeaderStructType = struct iphdr;
    
    enum class ProtocolType {
        TCP,
        UDP,
        ICMP,
        Undefined,
    };

    enum class IpVersionType {
        V4, V6
    };

    explicit IpViewer(const EthernetViewer& ethernetViewer);
    explicit IpViewer(RawFrameViewType rawFrame);
    explicit IpViewer(ConstRawFrameViewType rawFrame);

    IpVersionType getVersion();
    ProtocolType getProtocol();
    std::string_view getProtocolAsStr();
    unsigned int getTypeOfService();
    unsigned int getHeaderLengthInBytes();
    unsigned int getTotalLength();
    unsigned int getId();
    unsigned int getFragmentOffset();
    unsigned int getTTL();
    unsigned int getCheckSum();
    std::string getSourceIpAddressAsStr();
    std::string getDestIpAddressAsStr();
  

    IpVersionType getVersion() const;
    ProtocolType getProtocol() const;
    std::string_view getProtocolAsStr() const;
    unsigned int getTypeOfService() const;
    unsigned int getHeaderLength() const;
    unsigned int getHeaderLengthInBytes() const;
    unsigned int getTotalLength() const;
    unsigned int getId() const;
    unsigned int getFragmentOffset() const;
    unsigned int getTTL() const;
    unsigned int getCheckSum() const;
    std::string getSourceIpAddressAsStr() const;
    std::string getDestIpAddressAsStr() const;
    ConstRawFrameViewType getAsRawFrame();
    ConstRawFrameViewType getAsRawFrame() const;

    std::ostream& operator<<(std::ostream& os) const;
    
private:
    HeaderStructType* m_ipFrame;
    ConstRawFrameViewType m_rawFrame;
};

std::ostream& operator<<(std::ostream& os, const IpViewer& ipViewer);

} //! namespace net::posix::low_level

#endif //! VS_IP_VIEWER_H