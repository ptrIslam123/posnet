#include "icmp_viewer.h"

#include "ethernet_viewer.h"

#include <arpa/inet.h>

namespace {

net::posix::low_level::IcmpViewer::PackageType ExtractPackageType(const unsigned char type)
{
    using PackageType = net::posix::low_level::IcmpViewer::PackageType;
    switch (type) {
        case ICMP_ECHO:
            return PackageType::EchoRequest;
        case ICMP_ECHOREPLY:
            return PackageType::EchoReply;
        case ICMP_DEST_UNREACH:
            return PackageType::Unreached;
        case ICMP_TIME_EXCEEDED:
            return PackageType::TimeExceeded;
        case ICMP_INFO_REQUEST:
            return PackageType::InfoRequest;
        case ICMP_INFO_REPLY:
            return PackageType::InfoReply;
        case ICMP_TIMESTAMP:
            return PackageType::TimeStamp;
        case ICMP_TIMESTAMPREPLY:
            return PackageType::TimeStampReply;
        case ICMP_PARAMETERPROB:
            return PackageType::ParameterProblem;
        case ICMP_ADDRESS:
            return PackageType::AddressMaskRequest;
        case ICMP_ADDRESSREPLY:
            return PackageType::AddressMaskReply;
        default:
            return PackageType::Undefined;
    }
}

net::posix::low_level::IcmpViewer::PackageCode 
ExtractPackageCode(const net::posix::low_level::IcmpViewer::PackageType type, const unsigned char code)
{
    using PackageCode = net::posix::low_level::IcmpViewer::PackageCode;
    using PackageType = net::posix::low_level::IcmpViewer::PackageType;
    switch (type) {
        case PackageType::EchoRequest:
            return PackageCode::None;
        case PackageType::EchoReply:
            return PackageCode::None;    
        case PackageType::Unreached: {
            switch (code) {
                case ICMP_NET_UNREACH: return PackageCode::NetUnreachable;
                case ICMP_HOST_UNREACH: return PackageCode::HostUnreachable;
                case ICMP_PROT_UNREACH: return PackageCode::ProtocolUnreachable;
                case ICMP_PORT_UNREACH: return PackageCode::PortUnreachable;
                case ICMP_FRAG_NEEDED: return PackageCode::FragmentationNeeded;
                case ICMP_SR_FAILED: return PackageCode::SourceRouteFailed;
                case ICMP_NET_UNKNOWN: return PackageCode::NetUnknown;
                case ICMP_HOST_UNKNOWN: return PackageCode::HostUnknown;
                case ICMP_HOST_ISOLATED: return PackageCode::HostIsolated;
                case ICMP_NET_ANO:  [[fallthrough]];
                case ICMP_HOST_ANO: [[fallthrough]];
                case ICMP_NET_UNR_TOS: [[fallthrough]];
                case ICMP_HOST_UNR_TOS: return PackageCode::None;
                case ICMP_PKT_FILTERED: return PackageCode::PacketFiltered;
                case ICMP_PREC_VIOLATION: return PackageCode::PrecedenceViolation;
                case ICMP_PREC_CUTOFF: return PackageCode::PrecedenceCutOff;
                default: 
                    return PackageCode::Undefined;
            }
            break;
        }
        case PackageType::Redirect: {
            switch (code) {
                case ICMP_REDIR_NET: return PackageCode::RedirectNet;
                case ICMP_REDIR_HOST: return PackageCode::RedirectHost;
                case ICMP_REDIR_NETTOS: return PackageCode::RedirectNetForTypeOfService;
                case ICMP_REDIR_HOSTTOS: return PackageCode::RedirectHostForTypeOfService;
                default:
                    return PackageCode::Undefined;
            }
        }
        case PackageType::TimeExceeded: {
            switch (code) {
                case ICMP_EXC_TTL: return PackageCode::TTLCountExceeded;
                case ICMP_EXC_FRAGTIME: return PackageCode::FragmentReassTimeExceeded;
                default:
                    return PackageCode::Undefined;
            }

        }
        case PackageType::InfoRequest: [[fallthrough]];
        case PackageType::InfoReply: [[fallthrough]];
        case PackageType::TimeStamp: [[fallthrough]];
        case PackageType::TimeStampReply: [[fallthrough]];
        case PackageType::ParameterProblem: [[fallthrough]];
        case PackageType::AddressMaskRequest: [[fallthrough]];
        case PackageType::AddressMaskReply: return PackageCode::None;
        default:
            return PackageCode::Undefined;
    }
}

std::string_view PackageTypeToStr(const net::posix::low_level::IcmpViewer::PackageType packageType)
{
    using PackageType = net::posix::low_level::IcmpViewer::PackageType;
    switch (packageType) {
        case PackageType::EchoRequest:
            return "Echo Request";
        case PackageType::EchoReply:
            return "Echo Reply";
        case PackageType::Unreached:
            return "Destination Unreachable";
        case PackageType::Redirect:
            return "Redirect";
        case PackageType::TimeExceeded:
            return "Time Exceeded";
        case PackageType::InfoRequest:
            return "Information Request";
        case PackageType::InfoReply:
            return "Information Reply";
        case PackageType::TimeStamp:
            return "Timestamp Request";
        case PackageType::TimeStampReply:
            return "Timestamp Reply";
        case PackageType::ParameterProblem:
            return "Parameter Problem";
        case PackageType::AddressMaskRequest:
            return "Address Mask Request";
        case PackageType::AddressMaskReply:
            return "Address Mask Reply";
        default:
            return "Unknown";
    }
}

std::string_view PackageCodeToStr(const net::posix::low_level::IcmpViewer::PackageCode packageCode)
{
    using PackageCode = net::posix::low_level::IcmpViewer::PackageCode;
    switch (packageCode) {
        case PackageCode::NetUnreachable:
            return "Network Unreachable";
        case PackageCode::HostUnreachable:
            return "Host Unreachable";
        case PackageCode::ProtocolUnreachable:
            return "Protocol Unreachable";
        case PackageCode::PortUnreachable:
            return "Port Unreachable";
        case PackageCode::FragmentationNeeded:
            return "Fragmentation Needed and Don't Fragment was Set";
        case PackageCode::SourceRouteFailed:
            return "Source Route Failed";
        case PackageCode::NetUnknown:
            return "Network Unknown";
        case PackageCode::HostUnknown:
            return "Host Unknown";
        case PackageCode::HostIsolated:
            return "Host Isolated";
        case PackageCode::PacketFiltered:
            return "Packet Filtered";
        case PackageCode::PrecedenceViolation:
            return "Precedence Violation";
        case PackageCode::PrecedenceCutOff:
            return "Precedence Cut-off In Effect";
        case PackageCode::RedirectNet:
            return "Redirect Network";
        case PackageCode::RedirectHost:
            return "Redirect Host";
        case PackageCode::RedirectNetForTypeOfService:
            return "Redirect Network for Type of Service";
        case PackageCode::RedirectHostForTypeOfService:
            return "Redirect Host for Type of Service";
        case PackageCode::TTLCountExceeded:
            return "TTL Count Exceeded";
        case PackageCode::FragmentReassTimeExceeded:
            return "Fragment Reassembly Time Exceeded";
        case PackageCode::None:
            return "None";
        default:
            return "Unknown";
    }
}

} //! namespace

namespace net::posix::low_level {
    
IcmpViewer::IcmpViewer(const IpViewer& ipViewer):
m_icmpFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        ipViewer.getAsRawFrame().data() + ipViewer.getHeaderLengthInBytes() + sizeof(EthernetViewer::HeaderStructType)))),
m_rawFrame(ipViewer.getAsRawFrame())
{}

IcmpViewer::IcmpViewer(const ConstRawFrameViewType rawFrame):
m_icmpFrame(reinterpret_cast<HeaderStructType*>(
    const_cast<RawFrameViewType::value_type*>(
        rawFrame.data() + IpViewer(rawFrame).getHeaderLengthInBytes() + sizeof(EthernetViewer::HeaderStructType)))),
m_rawFrame(rawFrame)
{}

IcmpViewer::IcmpViewer(const RawFrameViewType rawFrame):
m_icmpFrame(reinterpret_cast<HeaderStructType*>(
    rawFrame.data() + IpViewer(rawFrame).getHeaderLengthInBytes() + sizeof(EthernetViewer::HeaderStructType))),
m_rawFrame(rawFrame)
{}

IcmpViewer::PackageType IcmpViewer::getType()
{
    return ExtractPackageType(m_icmpFrame->type);
}

std::string_view IcmpViewer::getTypeAsStr()
{
    return PackageTypeToStr(getType());
}

IcmpViewer::PackageCode IcmpViewer::getCode()
{
    return ExtractPackageCode(getType(), m_icmpFrame->code);
}

std::string_view IcmpViewer::getCodeAsStr()
{
    return PackageCodeToStr(getCode());
}

int IcmpViewer::getCheckSum()
{
    return ntohs(m_icmpFrame->checksum);
}

int IcmpViewer::getId()
{
    return ntohs(m_icmpFrame->un.echo.id);
}

int IcmpViewer::getSequenceNumber()
{
    return ntohs(m_icmpFrame->un.echo.sequence);
}

IcmpViewer::PackageType IcmpViewer::getType() const
{
    return ExtractPackageType(m_icmpFrame->type);
}

std::string_view IcmpViewer::getTypeAsStr() const
{
    return PackageTypeToStr(getType());
}

IcmpViewer::PackageCode IcmpViewer::getCode() const
{
    return ExtractPackageCode(getType(), m_icmpFrame->code);
}

std::string_view IcmpViewer::getCodeAsStr() const
{
    return PackageCodeToStr(getCode());
}

int IcmpViewer::getCheckSum() const
{
    return ntohs(m_icmpFrame->checksum);
}

int IcmpViewer::getId() const
{
    return ntohs(m_icmpFrame->un.echo.id);
}

int IcmpViewer::getSequenceNumber() const
{
    return ntohs(m_icmpFrame->un.echo.sequence);
}

std::ostream& IcmpViewer::operator<<(std::ostream& os) const
{
    os << "ICMP header {\n";
    os << "\ttype=" << getTypeAsStr() << "\n";
    os << "\tcode=" << getCodeAsStr() << "\n";
    os << "\tcheck-sum=" << getCheckSum() << "\n";
    os << "\tid=" << getId() << "\n";
    os << "\tsequence-number=" << getSequenceNumber() << "\n";
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const IcmpViewer& icmpViewer)
{
    return icmpViewer.operator<<(os);
}

} // namespace net::posix::low_level