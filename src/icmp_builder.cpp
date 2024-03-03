#include "include/frame-builder/icmp_builder.h"

#include "include/frame-viewers/icmp_viewer.h"

#include <cstring>

namespace {

unsigned char ExtractPackageType(const posnet::IcmpBuilder::PackageType type)
{
    using PackageType = posnet::IcmpBuilder::PackageType;
    constexpr unsigned char defaultValue = 0;
    switch (type) {
        case PackageType::EchoRequest:
            return ICMP_ECHO;
        case PackageType::EchoReply:
            return ICMP_ECHOREPLY;
        case PackageType::Unreached:
            return ICMP_DEST_UNREACH;
        case PackageType::TimeExceeded:
            return ICMP_TIME_EXCEEDED;
        case PackageType::InfoRequest:
            return ICMP_INFO_REQUEST;
        case PackageType::InfoReply:
            return ICMP_INFO_REPLY;
        case PackageType::TimeStamp:
            return ICMP_TIMESTAMP;
        case PackageType::TimeStampReply:
            return ICMP_TIMESTAMPREPLY;
        case PackageType::ParameterProblem:
            return ICMP_PARAMETERPROB;
        case PackageType::AddressMaskRequest:
            return ICMP_ADDRESS;
        case PackageType::AddressMaskReply:
            return ICMP_ADDRESSREPLY;
        default:
            return defaultValue;
    }
}

unsigned char ExtractPackageCode(const posnet::IcmpBuilder::PackageType type, const posnet::IcmpBuilder::PackageCode code)
{
    using PackageCode = posnet::IcmpBuilder::PackageCode;
    using PackageType = posnet::IcmpBuilder::PackageType;
    constexpr unsigned char defaultValue = 0;
    switch (type) {
        case PackageType::EchoRequest:
            return defaultValue;
        case PackageType::EchoReply:
            return defaultValue;    
        case PackageType::Unreached: {
            switch (code) {
                case PackageCode::NetUnreachable: return ICMP_NET_UNREACH;
                case PackageCode::HostUnreachable: return ICMP_HOST_UNREACH;
                case PackageCode::ProtocolUnreachable: return ICMP_PROT_UNREACH;
                case PackageCode::PortUnreachable: return ICMP_PORT_UNREACH;
                case PackageCode::FragmentationNeeded: return ICMP_FRAG_NEEDED;
                case PackageCode::SourceRouteFailed: return ICMP_SR_FAILED;
                case PackageCode::NetUnknown: return ICMP_NET_UNKNOWN;
                case PackageCode::HostUnknown: return ICMP_HOST_UNKNOWN;
                case PackageCode::HostIsolated: return ICMP_HOST_ISOLATED;
                case PackageCode::PacketFiltered: return ICMP_PKT_FILTERED;
                case PackageCode::PrecedenceViolation: return ICMP_PREC_VIOLATION;
                case PackageCode::PrecedenceCutOff: return ICMP_PREC_CUTOFF;
                default: 
                    return defaultValue;
            }
            break;
        }
        case PackageType::Redirect: {
            switch (code) {
                case PackageCode::RedirectNet: return ICMP_REDIR_NET;
                case PackageCode::RedirectHost: return ICMP_REDIR_HOST;
                case PackageCode::RedirectNetForTypeOfService: return ICMP_REDIR_NETTOS;
                case PackageCode::RedirectHostForTypeOfService: return ICMP_REDIR_HOSTTOS;
                default:
                    return defaultValue;
            }
        }
        case PackageType::TimeExceeded: {
            switch (code) {
                case PackageCode::TTLCountExceeded: return ICMP_EXC_TTL;
                case PackageCode::FragmentReassTimeExceeded: return ICMP_EXC_FRAGTIME;
                default:
                    return defaultValue;
            }

        }
        case PackageType::InfoRequest: [[fallthrough]];
        case PackageType::InfoReply: [[fallthrough]];
        case PackageType::TimeStamp: [[fallthrough]];
        case PackageType::TimeStampReply: [[fallthrough]];
        case PackageType::ParameterProblem: [[fallthrough]];
        case PackageType::AddressMaskRequest: [[fallthrough]];
        case PackageType::AddressMaskReply: [[fallthrough]];
        default:
            return defaultValue;
    }
}

} //! namespace

namespace posnet {

IcmpBuilder::IcmpBuilder():
BaseFrame(reinterpret_cast<const BaseFrame::ByteType*>(&m_frame), DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES),
m_frame()
{
    std::memset(&m_frame, 0 , DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES);
}

IcmpBuilder& IcmpBuilder::setType(const PackageType type) &
{
    m_frame.type = ExtractPackageType(type);
    return *this;
}

IcmpBuilder& IcmpBuilder::setCode(const PackageCode code) &
{
    m_frame.code = ExtractPackageCode(IcmpViewer(getAsRawFrameView()).getType(), code);
    return *this;
}

IcmpBuilder& IcmpBuilder::setCheckSum(const unsigned int checkSum) &
{
    m_frame.checksum = htons(checkSum);
    return *this;
}

IcmpBuilder& IcmpBuilder::setId(const unsigned int id) &
{
    m_frame.un.echo.sequence = htons(id);
    return *this;
}

IcmpBuilder& IcmpBuilder::setSequenceNumber(const unsigned int seqNumber) &
{
    m_frame.un.echo.sequence = htons(seqNumber);
    return *this;
}

std::ostream& IcmpBuilder::operator<<(std::ostream& os) const
{
    return os << IcmpViewer(getAsRawFrameView());
}

std::ostream& IcmpBuilder::operator<<(std::ostream& os)
{
    return os << IcmpViewer(getAsRawFrameView());
}

std::ostream& operator<<(std::ostream& os, const IcmpBuilder& icmpBuilder)
{
    return icmpBuilder.operator<<(os);
}

std::ostream& operator<<(std::ostream& os, IcmpBuilder& icmpBuilder)
{
    return icmpBuilder.operator<<(os);
}

} //! namespace posnet