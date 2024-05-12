#ifndef VS_ARP_BUILDER_H
#define VS_ARP_BUILDER_H

#include "include/base_frame.h"
#include "include/frame-viewers/arp_viewer.h"

#include <string>
#include <string_view>
#include <exception>
#include <ostream>

namespace posnet {

class BadArpPackage final : public std::exception {
public:
    explicit BadArpPackage(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

class ArpBuilder final : public BaseFrame {
public:
    /* Ethernet ARP packet from RFC 826 */
    struct arphdr final {
        uint16_t ar_hrd;		/* Format of hardware address.  */
        uint16_t ar_pro;		/* Format of protocol address.  */
        uint8_t ar_hln;		/* Length of hardware address.  */
        uint8_t ar_pln;		/* Length of protocol address.  */
        uint16_t ar_op;		/* ARP opcode (command).  */
        uint8_t arp_sha[ETH_ALEN];	/* sender hardware address */
        uint8_t arp_spa[4];		/* sender protocol(ip) address */
        uint8_t arp_tha[ETH_ALEN];	/* target hardware address */
        uint8_t arp_tpa[4];	        /* target protocol(ip) address */
        uint8_t padding[18];
    } __attribute__((packed));

    using RawFrameVieType = ArpViewer::RawFrameViewType;
    using ConstRawVieType = ArpViewer::ConstRawFrameViewType;
    using HeaderStructType = struct arphdr;
    using ProtocolType = ArpViewer::ProtocolType;
    using HardwareType = ArpViewer::HardwareType;
    using OpcodeType = ArpViewer::OpcodeType;

    static constexpr auto DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = sizeof(arphdr);
    static constexpr auto HARDWARE_LENGTH = 6;
    static constexpr auto PROTOCOL_LENGTH = 4;
    static constexpr auto DEFAULT_HARDWARE_ADDR_TYPE_LENGTH = def::MAC_ADDRESS_LENGTH_IN_BYTES;
    static constexpr auto DEFAULT_PROTOCOL_ADDR_TYPE_LENGTH = def::IP_ADDRESS_LENGTH_IN_BYTES;
    static constexpr auto DEFAULT_HARDWARE_ADDR_TYPE = HardwareType::EthernetHeader;
    static constexpr auto DEFAULT_PROTOCOL_ADDR_TYPE = ProtocolType::IP;

    explicit ArpBuilder();

    ArpBuilder& setHardwareType(HardwareType hardware) &;
    ArpBuilder& setProtocolType(ProtocolType protocol) &;
    ArpBuilder& setHardwareTypeLength(unsigned int length) &;
    ArpBuilder& setProtocolTypeLength(unsigned int length) &;
    ArpBuilder& setOpcodeType(OpcodeType opcode) &;
    ArpBuilder& setSourceMacAddress(std::string_view macAddr) &;
    ArpBuilder& setSourceIpAddress(std::string_view ipAddr) &;
    ArpBuilder& seTargetMacAddress(std::string_view macAddr) &;
    ArpBuilder& setTargetIpAddress(std::string_view ipAddr) &;

    std::ostream& operator<<(std::ostream& os) const;
    std::ostream& operator<<(std::ostream& os);

private:
    HeaderStructType m_frame;
};

std::ostream& operator<<(std::ostream& os, const ArpBuilder& arpBuilder);
std::ostream& operator<<(std::ostream& os, ArpBuilder& arpBuilder);

} //! namespace posnet

#endif //!VS_ARP_BUILDER_H