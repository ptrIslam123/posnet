#ifndef VS_ICMP_VIEWER_H
#define VS_ICMP_VIEWER_H

#include "ip_viewer.h"

#include <ostream>
#include <string>
#include <string_view>

#include <netinet/ip_icmp.h>

namespace posnet {

/**
 * @brief This class represents of ICMP frame.
 * @details Description of ICMP :
 * type: The type of the ICMP message. This field is 8 bits long and can take values such as ICMP_ECHO for an echo request or ICMP_ECHOREPLY for an echo reply.
 * 
 * code: The subtype of the ICMP message. This field is 8 bits long and is used to further specify the type of the ICMP message.
 * 
 * checksum: The checksum of the ICMP header and data. This field is 16 bits long.
 * 
 * un: A union that contains various fields depending on the type of the ICMP message. 
 * For example, for ICMP_ECHO and ICMP_ECHOREPLY, it contains the identifier and sequence number.
 */
class IcmpViewer final {
public:
    using RawFrameViewType = IpViewer::RawFrameViewType;
    using ConstRawFrameViewType = IpViewer::ConstRawFrameViewType;
    using HeaderStructType = struct icmphdr;

    enum class PackageType {
        EchoRequest,
        EchoReply,
        Unreached,
        Redirect,
        TimeExceeded,
        InfoRequest,
        InfoReply,
        TimeStamp,
        TimeStampReply,
        ParameterProblem,
        AddressMaskRequest,
        AddressMaskReply,
        Undefined,
    };

    enum class PackageCode {
        /* Codes for UNREACH. */
        NetUnreachable,
        HostUnreachable,
        ProtocolUnreachable,
        PortUnreachable,
        FragmentationNeeded,
        SourceRouteFailed,
        NetUnknown,
        HostUnknown,
        HostIsolated,
        PacketFiltered,
        PrecedenceViolation,
        PrecedenceCutOff,
        /* Codes for REDIRECT. */
        RedirectNet,
        RedirectHost,
        RedirectNetForTypeOfService,
        RedirectHostForTypeOfService,
        /* Codes for TIME_EXCEEDED. */
        TTLCountExceeded,
        FragmentReassTimeExceeded,

        None,
        Undefined
    };

    explicit IcmpViewer(const IpViewer& ipViewer);
    explicit IcmpViewer(RawFrameViewType rawFrame);
    explicit IcmpViewer(ConstRawFrameViewType rawFrame);

    PackageType getType();
    std::string_view getTypeAsStr();
    PackageCode getCode();
    std::string_view getCodeAsStr();
    int getCheckSum();
    int getId();
    int getSequenceNumber();

    PackageType getType() const;
    std::string_view getTypeAsStr() const;
    PackageCode getCode() const;
    std::string_view getCodeAsStr() const;
    int getCheckSum() const;
    int getId() const;
    int getSequenceNumber() const;

    std::ostream& operator<<(std::ostream& os) const;

private:
    HeaderStructType* m_icmpFrame;
    ConstRawFrameViewType m_rawFrame;
};

std::ostream& operator<<(std::ostream& os, const IcmpViewer& icmpViewer);

} // namespace posnet

#endif //! VS_ICMP_VIEWER_H