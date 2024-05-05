#ifndef VS_ARP_BUILDER_H
#define VS_ARP_BUILDER_H

#include "include/base_frame.h"
#include "include/frame-viewers/arp_viewer.h"

#include <ostream>

namespace posnet {

class ArpBuilder final : public BaseFrame {
public:
    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = ArpViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;
    static constexpr unsigned int HARDWARE_LENGTH = 6;
    static constexpr unsigned int PROTOCOL_LENGTH = 4;

    using RawFrameVieType = ArpViewer::RawFrameViewType;
    using ConstRawVieType = ArpViewer::ConstRawFrameViewType;
    using HeaderStructType = ArpViewer::HeaderStructType;
    using ProtocolType = ArpViewer::ProtocolType;
    using HardwareType = ArpViewer::HardwareType;
    using OpcodeType = ArpViewer::OpcodeType;

    explicit ArpBuilder();

    ArpBuilder& setHardwareType(HardwareType type) && = delete;
    ArpBuilder& setProtocolType(ProtocolType protocol) && = delete;
    ArpBuilder& setOpcode(OpcodeType opcode) && = delete;
    ArpBuilder& setSenderMacAddressAsStr(std::string_view senderMacAddr) && = delete;
    ArpBuilder& setTargetMacAddressAsStr(std::string_view targetMacAddr) && = delete;
    ArpBuilder& setSenderIpAddressAsStr(std::string_view senderIpAddr) && = delete;
    ArpBuilder& setTargetIpAddressAsStr(std::string_view targetIpAddr) && = delete;

    ArpBuilder& setHardwareLength(std::uint8_t length) &;
    ArpBuilder& setProtocolLength(std::uint8_t length) &;
    ArpBuilder& setHardwareType(HardwareType type) &;
    ArpBuilder& setProtocolType(ProtocolType protocol) &;
    ArpBuilder& setOpcode(OpcodeType opcode) &;
    ArpBuilder& setSenderMacAddressAsStr(std::string_view senderMacAddr) &;
    ArpBuilder& setTargetMacAddressAsStr(std::string_view targetMacAddr) &;
    ArpBuilder& setSenderIpAddressAsStr(std::string_view senderIpAddr) &;
    ArpBuilder& setTargetIpAddressAsStr(std::string_view targetIpAddr) &;

    std::ostream& operator<<(std::ostream& os) const;
    std::ostream& operator<<(std::ostream& os);

private:
    HeaderStructType m_frame;
};

std::ostream& operator<<(std::ostream& os, const ArpBuilder& arpBuilder);
std::ostream& operator<<(std::ostream& os, ArpBuilder& arpBuilder);

} //! namespace posnet

#endif //!VS_ARP_BUILDER_H