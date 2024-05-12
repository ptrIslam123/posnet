#ifndef VS_ARP_VIEWER_H
#define VS_ARP_VIEWER_H

#include "include/base_frame.h"
#include "include/frame-viewers/ethernet_viewer.h"

#include <string>
#include <string_view>
#include <ostream>
#include <optional>

namespace posnet {

/**
 * @brief This class represents of ARP frame.
 * @details Description of ARP layer: 
 * htype (Hardware Type): This field specifies the type of the hardware used. 
 * For example, Ethernet is typically represented as 1. Other types include Token Ring, FDDI, and others.
 * 
 * ptype (Protocol Type): This field specifies the type of the protocol. 
 * For example, IPv4 is typically represented as 0x0800. Other types include IPv6, AppleTalk, and others.
 * 
 * The hlen (Hardware Length) field specifies the length of the hardware address, and the plen (Protocol Length) field specifies the length of the protocol address.
 * 
 * The op (Operation) field indicates the type of ARP message. It is either a request (1) or a reply (2).
 * 
 * The sha (Sender Hardware Address) field contains the hardware address of the sender, and the spa (Sender Protocol Address) 
 * field contains the protocol address of the sender.
 * 
 * The tha (Target Hardware Address) field contains the hardware address of the intended receiver, and the tpa (Target Protocol Address) 
 * field contains the protocol address of the intended receiver.
 */
class ArpViewer final : public BaseFrame {
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

    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = sizeof(struct arphdr);
    
    using RawFrameViewType = EthernetViewer::RawFrameViewType;
    using ConstRawFrameViewType = EthernetViewer::ConstRawFrameViewType;
    using HeaderStructType = struct arphdr;

    enum class HardwareType {
        EthernetHeader,
    };

    enum class ProtocolType {
        IP
    };

    enum class OpcodeType {
        ArpRequest,
        ArpReply,
        RArpRequest,
        RArpReply,
        InArpRequest,
        InArpReply,
        Undefined,
    };

    explicit ArpViewer(EthernetViewer ethernetViewer);
    explicit ArpViewer(RawFrameViewType rawFrame);
    explicit ArpViewer(ConstRawFrameViewType rawFrame);

    // HardwareType getHardwareType();
    // std::string_view getHardwareTypeAsStr();
    // ProtocolType getProtocolType();
    // std::string_view getProtocolTypeAsStr();
    // OpcodeType getOpcode();
    // std::string_view getOpcodeAsStr();
    // std::string getSenderMacAddressAsStr();
    // std::string getTargetMacAddressAsStr();
    // std::string getSenderIpAddressAsStr();
    // std::string getTargetIpAddressAsStr();
    // std::uint8_t* getFrameHeaderStart();
    
    // HardwareType getHardwareType() const;
    // std::string_view getHardwareTypeAsStr() const;
    // ProtocolType getProtocolType() const;
    // std::string_view getProtocolTypeAsStr() const;
    // OpcodeType getOpcode() const;
    // std::string_view getOpcodeAsStr() const;
    // std::string getSenderMacAddressAsStr() const;
    // std::string getTargetMacAddressAsStr() const;
    // std::string getSenderIpAddressAsStr() const;
    // std::string getTargetIpAddressAsStr() const;

    //std::ostream& operator<<(std::ostream& os) const;

    static std::optional<uint16_t> HardwareTypeToNative(HardwareType hardware);
    static std::optional<uint16_t> ProtocolTypeToNative(ProtocolType protocol);
    static std::optional<uint16_t> OpcodeTypeToNative(OpcodeType opcode);

private:
    HeaderStructType* m_frame;
};

//std::ostream& operator<<(std::ostream& os, const ArpViewer& arpViewer);

} //! namespace posnet

#endif //! VS_ARP_VIEWER_H