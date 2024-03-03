#ifndef VS_UDP_BUILDER_H
#define VS_UDP_BUILDER_H

#include "include/base_frame.h"
#include "include/frame-viewers/udp_viewer.h"

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdint>

namespace posnet {
    
class UdpBuilder final : public BaseFrame {
public:
    static constexpr auto DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = UdpViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;

    using RawFrameViewType = UdpViewer::RawFrameViewType;
    using ConstRawFrameViewType = UdpViewer::ConstRawFrameViewType;
    using HeaderStructType = UdpViewer::HeaderStructType;
    using PortType = UdpViewer::PortType;

    explicit UdpBuilder();

    UdpBuilder& setSourcePort(PortType port) && = delete;
    UdpBuilder& setDestPort(PortType port) && = delete;
    UdpBuilder& setUdpDataGramLength(unsigned int length) && = delete;
    UdpBuilder& setCheckSum(unsigned int checkSum) && = delete;

    UdpBuilder& setSourcePort(PortType port) &;
    UdpBuilder& setDestPort(PortType port) &;
    UdpBuilder& setUdpDataGramLength(unsigned int length) &;
    UdpBuilder& setCheckSum(unsigned int checkSum) &;

    unsigned int getDefaultCheckSum();
    unsigned int getDefaultCheckSum() const;

    std::ostream& operator<<(std::ostream& os) const;
    std::ostream& operator<<(std::ostream& os);

private:
    HeaderStructType m_frame;
};

std::ostream& operator<<(std::ostream& os, const UdpBuilder& udpBuilder);
std::ostream& operator<<(std::ostream& os, UdpBuilder& udpBuilder);

UdpBuilder MakeDefaultUdpBuilder();

} // namespace posnet

#endif //! VS_UDP_BUILDER_H