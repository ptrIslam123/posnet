#include "frame-builder/arp_builder.h"

#include "utils/sock_addr_convertor.h"

//#include <net/if_arp.h>
#include <netinet/in.h>

#include <cstring>

namespace {

std::optional<std::uint16_t> ExtractHardwareType(const posnet::ArpViewer::HardwareType hardware)
{
    using HardwareType = posnet::ArpViewer::HardwareType;
    switch (hardware) {
        case HardwareType::ARP: return 1;
        case HardwareType::RARP: return 0;
        default:
            return std::nullopt;
    }
}

std::optional<std::uint16_t> ExtractProtocolType(const posnet::ArpViewer::ProtocolType protocol)
{
    using ProtocolType = posnet::ArpViewer::ProtocolType;
    switch (protocol) {
        case ProtocolType::V4: return ETH_P_IP;
        case ProtocolType::V6: 
        default: 
            return std::nullopt;
    }
}

std::optional<std::uint16_t> ExtractOpcodeType(const posnet::ArpViewer::OpcodeType opcode)
{
    using OpcodeType = posnet::ArpViewer::OpcodeType;
    switch (opcode) {
        case OpcodeType::ArpRequest: return ARPOP_REQUEST;
        case OpcodeType::ArpReply: return ARPOP_REPLY;
        case OpcodeType::RArpRequest: return ARPOP_RREQUEST;
        case OpcodeType::RArpReply: return ARPOP_RREPLY;
        case OpcodeType::InArpRequest: return ARPOP_InREQUEST;
        case OpcodeType::InArpReply: return ARPOP_InREPLY;
        default:
            return std::nullopt;
    }
}

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
    const auto typeValue = ExtractHardwareType(type);
    if (typeValue) {
        m_frame.hardwareType = htons(*typeValue);
    } else {
        throw std::runtime_error("");
    }

    return *this;
}

ArpBuilder& ArpBuilder::setProtocolType(const ProtocolType protocol) &
{
    const auto protocolValue =  ExtractProtocolType(protocol);
    if (protocolValue) {
        m_frame.protoType = htons(*protocolValue);
    } else {
        throw std::runtime_error("");
    }

    return *this;
}

ArpBuilder& ArpBuilder::setOpcode(const OpcodeType opcode) &
{
    const auto opcodeValue = ExtractOpcodeType(opcode);
    if (opcodeValue) {
        m_frame.opcode = htons(*opcodeValue);
    } else {
        throw std::runtime_error("");
    }

    return *this;
}

ArpBuilder& ArpBuilder::setSenderMacAddressAsStr(const std::string_view senderMacAddr) &
{
    auto macAddr = utils::StrToMacAddr(senderMacAddr);
    if (macAddr) {
        std::memcpy(m_frame.senderMac, macAddr->data(), macAddr->size());
    } else {
        throw std::runtime_error("");
    }

    return *this;
}

ArpBuilder& ArpBuilder::setTargetMacAddressAsStr(const std::string_view targetMacAddr) &
{
    auto macAddr = utils::StrToMacAddr(targetMacAddr);
    if (macAddr) {
        std::memcpy(m_frame.targetMac, macAddr->data(), macAddr->size());
    } else {
        throw std::runtime_error("");
    }

    return *this;
}

ArpBuilder& ArpBuilder::setSenderIpAddressAsStr(const std::string_view senderIpAddr) &
{
    auto ipAddr = utils::StrToIpAddrArray(senderIpAddr);
    if (ipAddr) {
        std::memcpy(m_frame.senderIp, ipAddr->data(), ipAddr->size());
    } else {
        throw std::runtime_error("");
    }

    return *this;
}

ArpBuilder& ArpBuilder::setTargetIpAddressAsStr(const std::string_view targetIpAddr) &
{
    auto ipAddr = utils::StrToIpAddrArray(targetIpAddr);
    if (ipAddr) {
        std::memcpy(m_frame.targetIp, ipAddr->data(), ipAddr->size());
    } else {
        throw std::runtime_error("");
    }

    return *this;
}

ArpBuilder& ArpBuilder::setHardwareLength(const std::uint8_t length) &
{
    m_frame.hardwareLen = htons(length);
    return *this;
}

ArpBuilder& ArpBuilder::setProtocolLength(const std::uint8_t length) &
{
    m_frame.protoLen = htons(length);
    return *this;    
}

std::ostream& ArpBuilder::operator<<(std::ostream& os) const
{
    return os << ArpViewer(getAsRawFrameView());
}

std::ostream& ArpBuilder::operator<<(std::ostream& os)
{
    return os << ArpViewer(getAsRawFrameView());
}

std::ostream& operator<<(std::ostream& os, const ArpBuilder& arpBuilder)
{
    return arpBuilder.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, ArpBuilder& arpBuilder)
{
    return arpBuilder.operator<<(os);
}

} //! namespace posnet