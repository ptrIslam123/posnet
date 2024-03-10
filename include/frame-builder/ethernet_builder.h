#ifndef VS_ETHERNET_BUILDER_H
#define VS_ETHERNET_BUILDER_H

#include "include/base_frame.h"
#include "include/frame-viewers/ethernet_viewer.h"

#include <string_view>
#include <span>
#include <ostream>

#include <netinet/ether.h>

namespace posnet {
    
class EthernetBuilder final : public BaseFrame {
public:
    static constexpr unsigned int DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES = EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;

    using RawFrameVieType = EthernetViewer::RawFrameViewType;
    using ConstRawVieType = EthernetViewer::ConstRawFrameViewType;
    using HeaderStructType = EthernetViewer::HeaderStructType;
    using ProtocolType = EthernetViewer::ProtocolType;

    explicit EthernetBuilder();

    EthernetBuilder& setDestMacAddress(std::string_view macAddr) && = delete;
    EthernetBuilder& setSourceMacAddress(std::string_view macAddr) && = delete;
    EthernetBuilder& setProtocol(ProtocolType protocol) && = delete;

    EthernetBuilder& setDestMacAddress(std::string_view macAddr) &;
    EthernetBuilder& setSourceMacAddress(std::string_view macAddr) &;
    EthernetBuilder& setProtocol(ProtocolType protocol) &;

    std::ostream& operator<<(std::ostream& os) const;
    std::ostream& operator<<(std::ostream& os);

private:
    HeaderStructType m_frame;
};

std::ostream& operator<<(std::ostream& os, const EthernetBuilder& ethernetBuilder);
std::ostream& operator<<(std::ostream& os, EthernetBuilder& ethernetBuilder);

} // namespace posnet


#endif //! VS_ETHERNET_BUILDER_H