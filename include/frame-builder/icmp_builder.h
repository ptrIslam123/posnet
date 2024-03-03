#ifndef VS_ICMP_BUILDER_H
#define VS_ICMP_BUILDER_H

#include "include/base_frame.h"
#include "include/frame-viewers/icmp_viewer.h"

#include <ostream>

namespace posnet {
    
class IcmpBuilder final : public BaseFrame {
public:
    static constexpr auto DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = IcmpViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;

    using RawFrameViewType = IcmpViewer::RawFrameViewType;
    using ConstRawFrameViewType = IcmpViewer::ConstRawFrameViewType;
    using HeaderStructType = IcmpViewer::HeaderStructType;
    using PackageType = IcmpViewer::PackageType;
    using PackageCode = IcmpViewer::PackageCode;

    explicit IcmpBuilder();
    IcmpBuilder& setType(PackageType type) && = delete;
    IcmpBuilder& setCode(PackageCode code) && = delete;
    IcmpBuilder& setCheckSum(unsigned int checkSum) && = delete;
    IcmpBuilder& setId(unsigned int id) && = delete;
    IcmpBuilder& setSequenceNumber(unsigned int seqNumber) && = delete;

    IcmpBuilder& setType(PackageType type) &;
    IcmpBuilder& setCode(PackageCode code) &;
    IcmpBuilder& setCheckSum(unsigned int checkSum) &;
    IcmpBuilder& setId(unsigned int id) &;
    IcmpBuilder& setSequenceNumber(unsigned int seqNumber) &;

    std::ostream& operator<<(std::ostream& os) const;
    std::ostream& operator<<(std::ostream& os);

private:
    HeaderStructType m_frame;
};

std::ostream& operator<<(std::ostream& os, const IcmpBuilder& icmpBuilder);
std::ostream& operator<<(std::ostream& os, IcmpBuilder& icmpBuilder);

} //! namespace posnet


#endif //! VS_ICMP_BUILDER_H