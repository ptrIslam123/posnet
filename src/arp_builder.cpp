#include "frame-builder/arp_builder.h"

#include "utils/sock_addr_convertor.h"

//#include <net/if_arp.h>
#include <netinet/in.h>

#include <cstring>

#define THROW(msg) (throw BadArpPackage(msg));

namespace {

} //! namespace

namespace posnet {

BadArpPackage::BadArpPackage(const std::string_view msg):
m_msg(msg)
{}

const char* BadArpPackage::what() const noexcept
{
    return m_msg.data();
}

ArpBuilder::ArpBuilder():
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(&m_frame), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame()
{
    std::memset(&m_frame, 0 , DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES);
}

ArpBuilder& ArpBuilder::setHardwareType(const HardwareType type) &
{
    const auto value = ArpViewer::HardwareTypeToNative(type);
    if (!value) {
        THROW("Unknown hardware address type")
    }

    m_frame.ar_hrd = htons(*value);
    return *this;
}

ArpBuilder& ArpBuilder::setProtocolType(const ProtocolType protocol) &
{
    const auto value = ArpViewer::ProtocolTypeToNative(protocol);
    if (!value) {
        THROW("Unknown protocol address type")
    }

    m_frame.ar_pro = htons(*value);
    return *this;
}

ArpBuilder& ArpBuilder::setOpcodeType(const OpcodeType opcode) &
{
    const auto value = ArpViewer::OpcodeTypeToNative(opcode);
    if (!value) {
        THROW("Unknown opcode type")
    }

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
    if (!addr) {
        THROW("Bad source mac address format=" + std::string(macAddr))
    }

    std::memcpy(m_frame.arp_sha, addr->data(), addr->size());
    return *this;
}

ArpBuilder& ArpBuilder::setSourceIpAddress(const std::string_view ipAddr) &
{
    const auto addr = utils::StrToIpAddrArray(ipAddr);
    if (!addr) {
        THROW("Bad source ip address format=" + std::string(ipAddr))
    }

    std::memcpy(m_frame.arp_spa, addr->data(), addr->size());
    return *this;
}

ArpBuilder& ArpBuilder::seTargetMacAddress(const std::string_view macAddr) &
{
    const auto addr = utils::StrToMacAddr(macAddr);
    if (!addr) {
        THROW("Bad target mac address format=" + std::string(macAddr))
    }

    std::memcpy(m_frame.arp_tha, addr->data(), addr->size());
    return *this;
}

ArpBuilder& ArpBuilder::setTargetIpAddress(const std::string_view ipAddr) &
{
    const auto addr = utils::StrToIpAddrArray(ipAddr);
    if (!addr) {
        THROW("Bad target ip address format=" + std::string(ipAddr))
    }

    std::memcpy(m_frame.arp_tpa, addr->data(), addr->size());
    return *this;
}

std::ostream& ArpBuilder::operator<<(std::ostream& os) const
{
    return os; // << ArpViewer(getAsRawFrameView());
}

std::ostream& ArpBuilder::operator<<(std::ostream& os)
{
    return os; // << ArpViewer(getAsRawFrameView());
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