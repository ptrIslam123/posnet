#ifndef VS_ETHERNET_VIEWER_H
#define VS_ETHERNET_VIEWER_H

#include "include/base_frame.h"

#include <string>
#include <string_view>
#include <ostream>

#include <netinet/ether.h>

namespace posnet {

/**
 * @brief This class represents of Ethernet frame.
 * @details Description of Ethernet layer:
 * h_dest: This is an array of 6 bytes that represents the destination MAC address. 
 * It is used to identify the final destination of the packet.
 * 
 * h_source: This is an array of 6 bytes that represents the source MAC address. 
 * It is used to identify the sender of the packet.
 * 
 * h_proto: This is a 2-byte field that indicates the type of the encapsulated protocol. 
 * The value of this field is typically used to determine the type of the payload that follows the Ethernet header. 
 * For example, if the value is 0x0800, it indicates that the payload is an IP packet. If the value is 0x0806, it indicates that the payload is an ARP packet. 
 * Other values are used for other types of encapsulated protocols.
 */
class EthernetViewer final : public BaseFrame {
public:
    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = sizeof(struct ethhdr);
    using RawFrameViewType = BaseFrame::RawFrameViewType; 
    using ConstRawFrameViewType = BaseFrame::ConstRawFrameViewType;
    using HeaderStructType = struct ethhdr;

    enum class ProtocolType {
        IP,
        ARP,
        RARP,
        Undefined,
    };

    explicit EthernetViewer(ConstRawFrameViewType rawFrame);

    std::string getDestMacAddressAsStr();
    std::string getSourceMacAddressAsStr();
    ProtocolType getProtocol();
    std::string_view getProtocolAsStr();

    std::string getDestMacAddressAsStr() const;
    std::string getSourceMacAddressAsStr() const;
    ProtocolType getProtocol() const;
    std::string_view getProtocolAsStr() const;

    std::uint8_t* getFrameHeaderStart();

    std::ostream& operator<<(std::ostream& os) const;

private:
    HeaderStructType *m_frame;
};

std::ostream& operator<<(std::ostream& os, const EthernetViewer& ethernetViewer);

} //! namespace posnet

#endif //! VS_ETHERNET_VIEWER_H