#ifndef VS_ETHERNET_BUILDER_H
#define VS_ETHERNET_BUILDER_H

#include "src/frame-viewers/ethernet_viewer.h"

#include <string_view>
#include <span>

#include <netinet/ether.h>

namespace posnet {
    
class EthernetBuilder final {
public:
    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;

    using RawFrameVieType = EthernetViewer::RawFrameViewType;
    using ConstRawVieType = EthernetViewer::ConstRawFrameViewType;
    using HeaderStructType = EthernetViewer::HeaderStructType;
    using ProtocolType = EthernetViewer::ProtocolType;

    explicit EthernetBuilder(RawFrameVieType rawFrame);

    EthernetBuilder& setDestMacAddress(std::string_view macAddr) && = delete;
    EthernetBuilder& setSourceMacAddress(std::string_view macAddr) && = delete;
    EthernetBuilder& setProtocol(ProtocolType protocol) && = delete;

    EthernetBuilder& setDestMacAddress(std::string_view macAddr) &;
    EthernetBuilder& setSourceMacAddress(std::string_view macAddr) &;
    EthernetBuilder& setProtocol(ProtocolType protocol) &;

private:
    HeaderStructType* m_frame;
};

} // namespace posnet


#endif //! VS_ETHERNET_BUILDER_H