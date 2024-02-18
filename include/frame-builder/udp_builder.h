#ifndef VS_UDP_BUILDER_H
#define VS_UDP_BUILDER_H

#include "include/frame-viewers/udp_viewer.h"

#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdint>

namespace posnet {
    
class UdpBuilder final {
public:
    static constexpr auto DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = UdpViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;

    using RawFrameViewType = UdpViewer::RawFrameViewType;
    using ConstRawFrameViewType = UdpViewer::ConstRawFrameViewType;
    using HeaderStructType = UdpViewer::HeaderStructType;
    using PortType = UdpViewer::PortType;

    explicit UdpBuilder(RawFrameViewType rawFrame);

    UdpBuilder& setSourcePort(PortType port) && = delete;
    UdpBuilder& setDestPort(PortType port) && = delete;
    UdpBuilder& setUdpDataGramLength(unsigned int length) && = delete;
    UdpBuilder& setCheckSum(unsigned int checkSum) && = delete;
    UdpBuilder& setPayload(std::span<std::int8_t> payload) && = delete;

    UdpBuilder& setSourcePort(PortType port) &;
    UdpBuilder& setDestPort(PortType port) &;
    UdpBuilder& setUdpDataGramLength(unsigned int length) &;
    UdpBuilder& setCheckSum(unsigned int checkSum) &;
    UdpBuilder& setPayload(std::span<std::int8_t> payload) &;
    UdpBuilder& setPayload(std::span<std::uint8_t> payload) &;

    unsigned int getDefaultCheckSum();
    unsigned int getDefaultCheckSum() const;

private:
    HeaderStructType* m_frame;
    ConstRawFrameViewType m_rawFrame;
    std::size_t m_payloadSize;
};

} // namespace posnet

#endif //! VS_UDP_BUILDER_H