#ifndef VS_IP_BUILDER_H
#define VS_IP_BUILDER_H

#include "include/base_frame.h"
#include "include/frame-viewers/ip_viewer.h"

#include <string_view>
#include <ostream>

namespace posnet {
    
class IpBuilder final : public BaseFrame {
public:
    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = IpViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;
    static constexpr unsigned int DEFAULT_FRAME_TOS_VALUE = IpViewer::DEFAULT_FRAME_TOS_VALUE;
    static constexpr unsigned int MAX_FRAME_FRAGMENT_OFFSET_VALUE = IpViewer::MAX_FRAME_FRAGMENT_OFFSET_VALUE;
    static constexpr unsigned int DEFAULT_FRAME_FRAGMENT_OFFSET_VALUE = IpViewer::DEFAULT_FRAME_FRAGMENT_OFFSET_VALUE;
    static constexpr unsigned int DEFAULT_FRAME_FRAGMENT_FLAG_VALUE = IpViewer::DEFAULT_FRAME_FRAGMENT_FLAG_VALUE;
    static constexpr unsigned int DEFAULT_FRAME_TTL_VALUE = IpViewer::DEFAULT_FRAME_TTL_VALUE;
    static constexpr unsigned int DEFAULT_FRAME_ID_VALUE = IpViewer::DEFAULT_FRAME_ID_VALUE;

    static constexpr std::string_view LOCAL_HOST_IP_ADDRESS = IpViewer::LOCAL_HOST_IP_ADDRESS;

    using RawFrameVieType = IpViewer::RawFrameViewType;
    using ConstRawVieType = IpViewer::ConstRawFrameViewType;
    using HeaderStructType = IpViewer::HeaderStructType;
    using ProtocolType = IpViewer::ProtocolType;
    using VersionType = IpViewer::VersionType;

    explicit IpBuilder();

    IpBuilder& setVersion(VersionType version) && = delete;
    IpBuilder& setProtocol(ProtocolType protocol) && = delete;
    IpBuilder& setTypeOfService(unsigned int tos = DEFAULT_FRAME_TOS_VALUE) && = delete;
    IpBuilder& setHeaderLengthInBytes(unsigned int length) && = delete;
    IpBuilder& setTotalLength(unsigned int length) && = delete;
    IpBuilder& setId(unsigned int id) && = delete;
    IpBuilder& setFragmentOffset(
        bool needToFragmentPackage = DEFAULT_FRAME_FRAGMENT_FLAG_VALUE, 
        unsigned int offset = DEFAULT_FRAME_FRAGMENT_OFFSET_VALUE
    ) && = delete;
    IpBuilder& setTTL(unsigned int ttl) && = delete;
    IpBuilder& setCheckSum(unsigned int checkSum) && = delete;
    IpBuilder& setSourceIpAddress(std::string_view ipAddr) && = delete;
    IpBuilder& setDestIpAddress(std::string_view ipAddr) && = delete;

    IpBuilder& setVersion(VersionType version) &;
    IpBuilder& setProtocol(ProtocolType protocol) &;
    IpBuilder& setTypeOfService(unsigned int tos) &;
    IpBuilder& setHeaderLengthInBytes(unsigned int length = DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES) &;
    IpBuilder& setTotalLength(unsigned int length) &;
    IpBuilder& setId(unsigned int id = DEFAULT_FRAME_ID_VALUE) &;
    IpBuilder& setFragmentOffset(
        bool needToFragmentPackage = DEFAULT_FRAME_FRAGMENT_FLAG_VALUE, 
        unsigned int offset = DEFAULT_FRAME_FRAGMENT_OFFSET_VALUE
    ) &;
    IpBuilder& setTTL(unsigned int ttl = DEFAULT_FRAME_TTL_VALUE) &;
    IpBuilder& setCheckSum(unsigned int checkSum) &;
    IpBuilder& setSourceIpAddress(std::string_view ipAddr) &;
    IpBuilder& setDestIpAddress(std::string_view ipAddr) &;

    unsigned int getDefaultCheckSum();
    unsigned int getDefaultCheckSum() const;

    std::ostream& operator<<(std::ostream& os) const;
    std::ostream& operator<<(std::ostream& os);

private:
    HeaderStructType m_frame;
};

std::ostream& operator<<(std::ostream& os, const IpBuilder& ipBuilder);
std::ostream& operator<<(std::ostream& os, IpBuilder& ipBuilder);

IpBuilder MakeDefaultIpBuilder();

} //! namespace posnet


#endif //! VS_IP_BUILDER_H