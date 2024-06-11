#include "frame-builder/arp_builder.h"

#include "utils/sock_addr_convertor.h"
#include "utils/assert.h"

#include <netinet/in.h>
#include <cstring>

namespace {

} //! namespace

namespace posnet {

ArpBuilder::ArpBuilder():
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(&m_frame), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame()
{
    std::memset(&m_frame, 0 , DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES);
}

ArpBuilder& ArpBuilder::setHardwareType(const HardwareType type) &
{
    const auto value = ArpBuilder::HardwareTypeToNative(type);
    ASSERTION(value, BadArpPackage, "Unknown hardware address type")
    m_frame.ar_hrd = htons(*value);
    return *this;
}

ArpBuilder& ArpBuilder::setProtocolType(const ProtocolType protocol) &
{
    const auto value = ArpBuilder::ProtocolTypeToNative(protocol);
    ASSERTION(value, BadArpPackage, "Unknown protocol address type")
    m_frame.ar_pro = htons(*value);
    return *this;
}

ArpBuilder& ArpBuilder::setOpcodeType(const OpcodeType opcode) &
{
    const auto value = ArpBuilder::OpcodeTypeToNative(opcode);
    ASSERTION(value, BadArpPackage, "Unknown opcode type")
    m_frame.ar_op = htons(*value);
    return *this;
}

ArpBuilder& ArpBuilder::setHardwareTypeLength(const unsigned int length) &
{
    m_frame.ar_hln = length;
    return *this;
}

ArpBuilder& ArpBuilder::setProtocolTypeLength(const unsigned int length) &
{
    m_frame.ar_pln = length;
    return *this;
}

ArpBuilder& ArpBuilder::setSourceMacAddress(const std::string_view macAddr) &
{
    const auto addr = utils::StrToMacAddr(macAddr);
    ASSERTION(addr, BadArpPackage, "Bad source mac address format=" + std::string(macAddr))
    std::memcpy(m_frame.arp_sha, addr->data(), addr->size());
    return *this;
}

ArpBuilder& ArpBuilder::setSourceIpAddress(const std::string_view ipAddr) &
{
    const auto addr = utils::v4::StrToIpAddrArray(ipAddr);
    ASSERTION(addr, BadArpPackage, "Bad source ip address format=" + std::string(ipAddr))
    std::memcpy(m_frame.arp_spa, addr->data(), addr->size());
    return *this;
}

ArpBuilder& ArpBuilder::seTargetMacAddress(const std::string_view macAddr) &
{
    const auto addr = utils::StrToMacAddr(macAddr);
    ASSERTION(addr, BadArpPackage, "Bad target mac address format=" + std::string(macAddr))
    std::memcpy(m_frame.arp_tha, addr->data(), addr->size());
    return *this;
}

ArpBuilder& ArpBuilder::setTargetIpAddress(const std::string_view ipAddr) &
{
    const auto addr = utils::v4::StrToIpAddrArray(ipAddr);
    ASSERTION(addr, BadArpPackage, "Bad target ip address format=" + std::string(ipAddr))
    std::memcpy(m_frame.arp_tpa, addr->data(), addr->size());
    return *this;
}

std::ostream& ArpBuilder::operator<<(std::ostream& os)
{
    return os << ArpViewer(getAsRawFrameView());
}

std::ostream& operator<<(std::ostream& os, ArpBuilder& arpBuilder)
{
    return arpBuilder.operator<<(os);
}

std::optional<uint16_t> ArpBuilder::HardwareTypeToNative(const HardwareType hardware)
{
    switch (hardware) {
        case HardwareType::EthernetHeader : return ARPHRD_ETHER;
        default: return std::nullopt;
    }
}

std::optional<uint16_t> ArpBuilder::ProtocolTypeToNative(const ProtocolType protocol)
{
    switch (protocol) {
        case ProtocolType::IP: return ETHERTYPE_IP;
        default: return std::nullopt;
    }
}

std::optional<uint16_t> ArpBuilder::OpcodeTypeToNative(const OpcodeType opcode)
{
    switch (opcode) {
        case OpcodeType::ArpRequest: return ARPOP_REQUEST;
        case OpcodeType::ArpReply: return ARPOP_REPLY;
        case OpcodeType::RArpRequest : return ARPOP_RREQUEST;
        case OpcodeType::RArpReply: return ARPOP_RREPLY;
        case OpcodeType::InArpRequest: return ARPOP_InREQUEST;
        case OpcodeType::InArpReply: return ARPOP_InREPLY;
        default: return std::nullopt;
    }
}

} //! namespace posnet
