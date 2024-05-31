#include "frame-viewers/arp_viewer.h"

#include "utils/sock_addr_convertor.h"
#include "utils/assert.h"

#include <array>
#include <cstdio>

#include <arpa/inet.h>

namespace {
    
std::string_view GetHardwareAddrTypeAsStr(const posnet::ArpViewer::HardwareType hardware)
{
    switch (hardware) {
        using Type = posnet::ArpViewer::HardwareType;
        case Type::EthernetHeader: {
            return std::string_view{"Ethernet"};
        }
        default: {
            return std::string_view{"Unknown"};
        }
    }
}

std::string_view GetProtocolAddrTypeAsStr(const posnet::ArpViewer::ProtocolType protocol)
{
    switch (protocol) {
        using Type = posnet::ArpViewer::ProtocolType;
        case Type::IP: {
            return std::string_view{"IP"};
        }
        default: {
            return std::string_view{"Unknown"};
        }
    }
}

std::string_view GetOpcodeTypeAsStr(const posnet::ArpViewer::OpcodeType opcode)
{
    switch (opcode) {
        using Type = posnet::ArpViewer::OpcodeType;
        case Type::ArpRequest: return "Arp Request";
        case Type::ArpReply: return "Arp Reply";
        case Type::RArpRequest: return "Revers Arp Request";
        case Type::RArpReply: return "Revers Arp Reply";
        case Type::InArpRequest: return "InArpRequest";
        case Type::InArpReply: return "InArpReply";
        default: return "Unknown";
    }
}

} // namespace


namespace posnet {

BadArpPackage::BadArpPackage(const std::string_view msg):
m_msg(msg)
{}

const char* BadArpPackage::what() const noexcept
{
    return m_msg.data();
}

ArpViewer::ArpViewer(EthernetViewer ethernetViewer):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(ethernetViewer.getStart()), ethernetViewer.getSize()),
m_frame(reinterpret_cast<HeaderStructType*>(
    const_cast<ByteType*>(ethernetViewer.getStart() + EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES)))
{}

ArpViewer::ArpViewer(const RawFrameViewType rawFrame):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(rawFrame.data()), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame(reinterpret_cast<HeaderStructType*>(rawFrame.data()))
{}

ArpViewer::ArpViewer(const ConstRawFrameViewType rawFrame):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(rawFrame.data()), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(rawFrame.data())))
{}

ArpViewer::HardwareType ArpViewer::getHardwareAddrType()
{
    const auto hwdType = ArpViewer::NativeToHardwareType(ntohs(m_frame->ar_hrd));
    ASSERTION(hwdType, BadArpPackage, "Invalid native hardware address type")
    return *hwdType;
}

ArpViewer::ProtocolType ArpViewer::getProtocolAddrType()
{
    const auto protoType = ArpViewer::NativeToProtocolType(ntohs(m_frame->ar_pro));
    ASSERTION(protoType, BadArpPackage, "Invalid native protocol address type")
    return *protoType;
}

std::string_view ArpViewer::getProtocolAddrTypeAsStr()
{
    return GetProtocolAddrTypeAsStr(getProtocolAddrType());
}

std::string_view ArpViewer::getHardwareAddrTypeAsStr()
{
    return GetHardwareAddrTypeAsStr(getHardwareAddrType());
}

std::string_view ArpViewer::getOpcodeTypeAsStr()
{
    return GetOpcodeTypeAsStr(getOpcodeType());
}

ArpViewer::SizeType ArpViewer::getHardwareAddrLength()
{
    return ntohs(m_frame->ar_hln);
}

ArpViewer::SizeType ArpViewer::getProtocolAddrLength()
{
    return ntohs(m_frame->ar_pln);
}

ArpViewer::OpcodeType ArpViewer::getOpcodeType()
{
    const auto opcode = ArpViewer::NativeToOpcodeType(ntohs(m_frame->ar_op));
    ASSERTION(opcode, BadArpPackage, "Invalid native opcode type")
    return *opcode;
}

std::string ArpViewer::getSenderMacAddressAsStr()
{
    auto addr = utils::MacAddrToStr(m_frame->arp_sha);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad sender mac addr format")
    return addr;
}

std::string ArpViewer::getSenderIpAddressAsStr()
{
    auto addr = utils::IpAddrToStr(m_frame->arp_spa);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad sender ip addr format")
    return addr;
}

std::string ArpViewer::getTargetMacAddressAsStr()
{
    auto addr = utils::MacAddrToStr(m_frame->arp_tha);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad target mac addr format")
    return addr;
}

std::string ArpViewer::getTargetIpAddressAsStr()
{
    auto addr = utils::IpAddrToStr(m_frame->arp_tpa);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad target ip addr format")
    return addr;
}

ArpViewer::HardwareType ArpViewer::getHardwareAddrType() const
{
    const auto hwdType = ArpViewer::NativeToHardwareType(ntohs(m_frame->ar_hrd));
    ASSERTION(hwdType, BadArpPackage, "Invalid native hardware address type")
    return *hwdType;
}

ArpViewer::ProtocolType ArpViewer::getProtocolAddrType() const
{
    const auto protoType = ArpViewer::NativeToProtocolType(ntohs(m_frame->ar_pro));
    ASSERTION(protoType, BadArpPackage, "Invalid native protocol address type")
    return *protoType;
}

ArpViewer::SizeType ArpViewer::getHardwareAddrLength() const
{
    return ntohs(m_frame->ar_hln);
}

ArpViewer::SizeType ArpViewer::getProtocolAddrLength() const
{
    return ntohs(m_frame->ar_pln);
}

ArpViewer::OpcodeType ArpViewer::getOpcodeType() const
{
    const auto opcode = ArpViewer::NativeToOpcodeType(ntohs(m_frame->ar_op));
    ASSERTION(opcode, BadArpPackage, "Invalid native opcode type")
    return *opcode;
}

std::string ArpViewer::getSenderMacAddressAsStr() const
{
    auto addr = utils::MacAddrToStr(m_frame->arp_sha);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad sender mac addr format")
    return addr;
}

std::string ArpViewer::getSenderIpAddressAsStr() const
{
    auto addr = utils::IpAddrToStr(m_frame->arp_spa);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad sender ip addr format")
    return addr;
}

std::string ArpViewer::getTargetMacAddressAsStr() const
{
    auto addr = utils::MacAddrToStr(m_frame->arp_tha);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad target mac addr format")
    return addr;
}

std::string ArpViewer::getTargetIpAddressAsStr() const
{
    auto addr = utils::IpAddrToStr(m_frame->arp_tpa);
    ASSERTION(!addr.empty(), BadArpPackage, "Bad target ip addr format")
    return addr;
}

std::string_view ArpViewer::getProtocolAddrTypeAsStr() const
{
    return GetProtocolAddrTypeAsStr(getProtocolAddrType());
}

std::string_view ArpViewer::getHardwareAddrTypeAsStr() const
{
    return GetHardwareAddrTypeAsStr(getHardwareAddrType());
}

std::string_view ArpViewer::getOpcodeTypeAsStr() const
{
    return GetOpcodeTypeAsStr(getOpcodeType());
}


std::ostream& ArpViewer::operator<<(std::ostream& os) const
{
    os << "Arp header {" << "\n";
    os << "\t sender-mac-addr=" << getSenderMacAddressAsStr() << "\n";
    os << "\t sender-ip-addr=" << getSenderIpAddressAsStr() << "\n";
    os << "\t target-mac-addr=" << getTargetMacAddressAsStr() << "\n";
    os << "\t target-ip-addr=" << getTargetIpAddressAsStr() << "\n";
    os << "}";
    return os;
}

std::optional<ArpViewer::HardwareType> ArpViewer::NativeToHardwareType(const uint16_t hardware)
{
    switch (hardware) {
        case ARPHRD_ETHER: return HardwareType::EthernetHeader;
        default: return std::nullopt;
    }
}

std::optional<ArpViewer::ProtocolType> ArpViewer::NativeToProtocolType(const uint16_t protocol)
{
    switch (protocol) {
        case ETHERTYPE_IP: return ProtocolType::IP;
        default: return std::nullopt;
    }
}

std::optional<ArpViewer::OpcodeType> ArpViewer::NativeToOpcodeType(const uint16_t opcode)
{
    switch (opcode) {
        case ARPOP_REQUEST: return OpcodeType::ArpRequest;
        case ARPOP_REPLY: return OpcodeType::ArpReply;
        case ARPOP_RREQUEST: return OpcodeType::RArpRequest;
        case ARPOP_RREPLY: return OpcodeType::RArpReply;
        case ARPOP_InREQUEST: return OpcodeType::InArpRequest;
        case ARPOP_InREPLY: return OpcodeType::InArpReply;
        default: return std::nullopt;
    }
}


std::ostream& operator<<(std::ostream& os, const ArpViewer& arpViewer)
{
    return arpViewer.operator<<(os);
}

} //! namespace posnet
