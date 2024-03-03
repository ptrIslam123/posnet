#ifndef VS_IP_VIEWER_H
#define VS_IP_VIEWER_H

#include "include/base_frame.h"
#include "include/frame-viewers/ethernet_viewer.h"

#include <string>
#include <string_view>
#include <ostream>

#include <netinet/ip.h>

namespace posnet {

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
class IpViewer final : public BaseFrame {
public:
    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = sizeof(struct iphdr);
    static constexpr unsigned int DEFAULT_FRAME_TOS_VALUE = 0;
    static constexpr unsigned int MAX_FRAME_FRAGMENT_OFFSET_VALUE = 8191;
    static constexpr unsigned int DEFAULT_FRAME_FRAGMENT_OFFSET_VALUE = 0;
    static constexpr unsigned int DEFAULT_FRAME_FRAGMENT_FLAG_VALUE = 0X4000; // (MF) More Fragments
    static constexpr unsigned int DEFAULT_FRAME_TTL_VALUE = 64;
    static constexpr unsigned int DEFAULT_FRAME_ID_VALUE = 0;

    static constexpr std::string_view LOCAL_HOST_IP_ADDRESS = "127.0.0.1";

    using RawFrameViewType = EthernetViewer::RawFrameViewType;
    using ConstRawFrameViewType = EthernetViewer::ConstRawFrameViewType;
    using HeaderStructType = struct iphdr;
    
    enum class ProtocolType {
        TCP,
        UDP,
        ICMP,
        Undefined,
    };

    enum class VersionType {
        V4, V6
    };

    explicit IpViewer(EthernetViewer ethernetViewer);
    explicit IpViewer(RawFrameViewType rawFrame);
    explicit IpViewer(ConstRawFrameViewType rawFrame);

    VersionType getVersion();
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
  
    VersionType getVersion() const;
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
    
    std::uint8_t* getFrameHeaderStart();

    std::ostream& operator<<(std::ostream& os) const;
    
private:
    HeaderStructType* m_frame;
};

std::ostream& operator<<(std::ostream& os, const IpViewer& ipViewer);

} //! namespace posnet

#endif //! VS_IP_VIEWER_H
