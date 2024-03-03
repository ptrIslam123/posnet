#include "frame-viewers/arp_viewer.h"

#include "utils/sock_addr_convertor.h"

#include <array>
#include <cstdio>

#include <arpa/inet.h>

namespace {

posnet::ArpViewer::HardwareType ExtractHardwareType(const uint16_t hardware)
{
    using HardwareType = posnet::ArpViewer::HardwareType;
    switch (hardware) {
        case 1: return HardwareType::ARP;
        case 0: return HardwareType::RARP;
        default:
            return HardwareType::Undefined;
    }
}

posnet::ArpViewer::ProtocolType ExtractProtocolType(const uint16_t protocol)
{
    using ProtocolType = posnet::ArpViewer::ProtocolType;
    return (protocol == 0x0800 ? ProtocolType::V4 : ProtocolType::V6);
}

posnet::ArpViewer::OpcodeType ExtractOpcodeType(const uint16_t opcode)
{
    using OpcodeType = posnet::ArpViewer::OpcodeType;
    switch (opcode) {
        case 1: return OpcodeType::ArpRequest;
        case 2: return OpcodeType::ArpReply;
        case 3: return OpcodeType::RArpRequest;
        case 4: return OpcodeType::RArpReply;
        case 8: return OpcodeType::InArpRequest;
        case 9: return OpcodeType::InArpReply;
        default:
            return OpcodeType::Undefined;
    }
}

std::string_view HardwareTypeToStr(const posnet::ArpViewer::HardwareType hardware)
{
    using HardwareType = posnet::ArpViewer::HardwareType;
    switch (hardware) {
        case HardwareType::ARP: return "ARP";
        case HardwareType::RARP: return "RARP";
        default:
            return "Undefined";
    }
}

std::string_view ProtocolTypeToStr(const posnet::ArpViewer::ProtocolType protocol)
{
    using ProtocolType = posnet::ArpViewer::ProtocolType;
    return (protocol == ProtocolType::V4 ? "V4" : "V6");
}

std::string_view OpcodeTypeToStr(const posnet::ArpViewer::OpcodeType opcode)
{
    using OpcodeType = posnet::ArpViewer::OpcodeType;
    switch (opcode) {
        case OpcodeType::ArpRequest: return "ArpRequest";
        case OpcodeType::ArpReply: return "ArpReply";
        case OpcodeType::RArpRequest: return "RArpRequest";
        case OpcodeType::RArpReply: return "RArpReply";
        case OpcodeType::InArpRequest: return "InArpRequest";
        case OpcodeType::InArpReply: return "InArpReply";
        default:
            return "Undefined";
    }
}

} //! namespace

namespace posnet {

ArpViewer::ArpViewer(EthernetViewer ethernetViewer):
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(ethernetViewer.getStart()), ethernetViewer.getSize()),
m_frame(reinterpret_cast<HeaderStructType*>(
    ethernetViewer.getFrameHeaderStart() + EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES))
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

ArpViewer::HardwareType ArpViewer::getHardwareType()
{
    return ExtractHardwareType(ntohs(m_frame->hardwareType));
}

std::string_view ArpViewer::getHardwareTypeAsStr()
{
    return HardwareTypeToStr(getHardwareType());
}

ArpViewer::ProtocolType ArpViewer::getProtocolType()
{
    return ExtractProtocolType(ntohs(m_frame->protoType));
}

std::string_view ArpViewer::getProtocolTypeAsStr()
{
    return ProtocolTypeToStr(getProtocolType());
}

ArpViewer::OpcodeType ArpViewer::getOpcode()
{
    return ExtractOpcodeType(ntohs(m_frame->opcode));
}

std::string_view ArpViewer::getOpcodeAsStr()
{
    return OpcodeTypeToStr(getOpcode());
}

std::string ArpViewer::getSenderMacAddressAsStr()
{
    return posnet::utils::MacAddrToStr(m_frame->senderMac);
}

std::string ArpViewer::getTargetMacAddressAsStr()
{
    return posnet::utils::MacAddrToStr(m_frame->targetMac);
}

std::string ArpViewer::getSenderIpAddressAsStr()
{
    return posnet::utils::IpAddrToStr(m_frame->senderIp);
}

std::string ArpViewer::getTargetIpAddressAsStr()
{
    return posnet::utils::IpAddrToStr(m_frame->targetIp);
}

ArpViewer::HardwareType ArpViewer::getHardwareType() const
{
    return ExtractHardwareType(ntohs(m_frame->hardwareType));
}

std::string_view ArpViewer::getHardwareTypeAsStr() const
{
    return HardwareTypeToStr(getHardwareType());
}

ArpViewer::ProtocolType ArpViewer::getProtocolType() const
{
    return ExtractProtocolType(ntohs(m_frame->protoType));
}

std::string_view ArpViewer::getProtocolTypeAsStr() const
{
    return ProtocolTypeToStr(getProtocolType());
}

ArpViewer::OpcodeType ArpViewer::getOpcode() const
{
    return ExtractOpcodeType(ntohs(m_frame->opcode));
}

std::string_view ArpViewer::getOpcodeAsStr() const
{
    return OpcodeTypeToStr(getOpcode());
}

std::string ArpViewer::getSenderMacAddressAsStr() const
{
    return posnet::utils::MacAddrToStr(m_frame->senderMac);
}

std::string ArpViewer::getTargetMacAddressAsStr() const
{
    return posnet::utils::MacAddrToStr(m_frame->targetMac);
}

std::string ArpViewer::getSenderIpAddressAsStr() const
{
    return posnet::utils::IpAddrToStr(m_frame->senderIp);
}

std::string ArpViewer::getTargetIpAddressAsStr() const
{
    return posnet::utils::IpAddrToStr(m_frame->targetIp);
}

std::uint8_t* ArpViewer::getFrameHeaderStart()
{
    return reinterpret_cast<std::uint8_t*>(m_frame);
}

std::ostream& ArpViewer::operator<<(std::ostream& os) const
{
     os << "ARP header {\n";
     os << "\thardware-type=" << getHardwareTypeAsStr() << "\n";
     os << "\tprotocol-type=" << getProtocolTypeAsStr() << "\n";
     os << "\topcode=" << getOpcodeAsStr() << "\n";
     os << "\tsender-ip-address=" << getSenderIpAddressAsStr() << "\n";
     os << "\ttarget-ip-address=" << getTargetIpAddressAsStr() << "\n";
     os << "\tsender-mac-address=" << getSenderMacAddressAsStr() << "\n";
     os << "\ttarget-mac-address=" << getSenderMacAddressAsStr() << "\n";
     os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ArpViewer& arpViewer)
{
    return arpViewer.operator<<(os);
}

} //! namespace posnet