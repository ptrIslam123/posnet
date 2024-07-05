#ifndef FRAME_CAPTURER_H
#define FRAME_CAPTURER_H

#include "include/base_frame.h"
#include "include/frame-filter/frame_filter.h"

#include <functional>
#include <exception>
#include <string>
#include <string_view>

namespace posnet {

class IpViewer;
class ArpViewer;
class EthernetViewer;

class BadFrameCapturer final : public std::exception {
public:
    explicit BadFrameCapturer(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

class FrameCapturer final {
public:
    using ConstRawFrameViewType = BaseFrame::ConstRawFrameViewType;
    using CallbackType = std::function<bool(
        const BaseFrame& baseFrame, 
        FrameFilter::ProtocolType protocol,
        int level
    )>;

    static constexpr auto CONTINUE_CAPTURING = true;
    static constexpr auto STOP_CAPTURING = !CONTINUE_CAPTURING;
    explicit FrameCapturer() = default;
    void capture(const FrameFilter& filter, CallbackType callback);

private:
    bool parseCapturedFrame(
        const ConstRawFrameViewType rawFrameBuffer, 
        const FrameFilter& filter,
        CallbackType callback
    );

    bool parseCapturedFrameOnL1(
        const ConstRawFrameViewType rawFrameBuffer, 
        const FrameFilter& filter,
        CallbackType callback
    );

    bool parseCapturedFrameOnL2(
        const EthernetViewer& ethernetViewer, 
        const FrameFilter& filter,
        CallbackType callback
    );

    bool parseCapturedFrameOnL3(
        const ArpViewer& arpViewer, 
        const FrameFilter& filter, 
        CallbackType callback
    );

    bool parseCapturedFrameOnL3(
        const IpViewer& ipViewer, 
        const FrameFilter& filter, 
        CallbackType callback
    );

    FrameFilter m_filter;
};

} //! namespace posnet

#endif //! FRAME_CAPTURER_H