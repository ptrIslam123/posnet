#ifndef VS_ICMP_VIEWER_H
#define VS_ICMP_VIEWER_H

#include "include/base_frame.h"
#include "include/frame-viewers/ip_viewer.h"

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
class IcmpViewer final : public BaseFrame {
public:
    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = sizeof(struct icmphdr);
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

    explicit IcmpViewer(IpViewer ipViewer);
    explicit IcmpViewer(RawFrameViewType rawFrame);
    explicit IcmpViewer(ConstRawFrameViewType rawFrame);

    PackageType getType();
    std::string_view getTypeAsStr();
    PackageCode getCode();
    std::string_view getCodeAsStr();
    unsigned int getCheckSum();
    unsigned int getId();
    unsigned int getSequenceNumber();

    PackageType getType() const;
    std::string_view getTypeAsStr() const;
    PackageCode getCode() const;
    std::string_view getCodeAsStr() const;
    unsigned int getCheckSum() const;
    unsigned int getId() const;
    unsigned int getSequenceNumber() const;

    std::ostream& operator<<(std::ostream& os) const;

private:
    HeaderStructType* m_frame;
};

std::ostream& operator<<(std::ostream& os, const IcmpViewer& icmpViewer);

} // namespace posnet

#endif //! VS_ICMP_VIEWER_H