#include "include/frame-capturer/frame_capturer.h"

#include "include/utils/assert.h"
#include "include/utils/scoped_lock.h"
#include "include/utils/system_error.h"

#include "include/frame-viewers/ethernet_viewer.h"
#include "include/frame-viewers/ip_viewer.h"
#include "include/frame-viewers/udp_viewer.h"
#include "include/frame-viewers/tcp_viewer.h"
#include "include/frame-viewers/icmp_viewer.h"
#include "include/frame-viewers/arp_viewer.h"

#include <array>

#include <unistd.h> // For close socket

namespace posnet {

BadFrameCapturer::BadFrameCapturer(const std::string_view msg):
m_msg(msg)
{}

const char* BadFrameCapturer::what() const noexcept
{
    return m_msg.data();
}

bool FrameCapturer::parseCapturedFrameOnL3(
    const IpViewer& ipViewer, 
    const FrameFilter& filter,
    CallbackType callback
) {
    constexpr auto level = 3;
    switch (ipViewer.getProtocol()) {
        using ProtocolType = posnet::IpViewer::ProtocolType;
        case ProtocolType::TCP: {
            const posnet::TcpViewer tcpViewer{ ipViewer };
            if (!filter.filter(tcpViewer)) {
                return callback(tcpViewer, FrameFilter::ProtocolType::TCP, level);
            }
            break;
        }
        case ProtocolType::UDP: {
            const posnet::UdpViewer udpViewer{ ipViewer };
            if (!filter.filter(udpViewer)) {
                return callback(udpViewer, FrameFilter::ProtocolType::UDP, level);
            }
            break;
        }
        case ProtocolType::ICMP: {
            const posnet::IcmpViewer icmpViewer{ ipViewer };
            if (!filter.filter(icmpViewer)) {
                return callback(icmpViewer, FrameFilter::ProtocolType::ICMP, level);
            }
            break;
        }
        default: {
            //! Do nothing
        }
    }

    return CONTINUE_CAPTURING;
}

bool FrameCapturer::parseCapturedFrameOnL3(
    const ArpViewer& arpViewer, 
    const FrameFilter& filter, 
    CallbackType callback
) {
    constexpr auto level = 3;
    if (!filter.filter(arpViewer)) {
        return callback(arpViewer, FrameFilter::ProtocolType::ARP, level);
    }

    return CONTINUE_CAPTURING;
}

bool FrameCapturer::parseCapturedFrameOnL2(
    const EthernetViewer& ethernetViewer, 
    const FrameFilter& filter,
    CallbackType callback
) {
    switch (ethernetViewer.getProtocol()) {
        using ProtocolType = posnet::EthernetViewer::ProtocolType; 
        case ProtocolType::IP: {
            const posnet::IpViewer ipViewer{ ethernetViewer };
            if (!filter.filter(ipViewer)) {
                return parseCapturedFrameOnL3(ipViewer, filter, callback);
            }
            break;
        }
        case ProtocolType::RARP:
        case ProtocolType::ARP: {
            const posnet::ArpViewer arpViewer{ ethernetViewer };
            if (!filter.filter(arpViewer)) {
                return parseCapturedFrameOnL3(arpViewer, filter, callback);
            }
            break;
        }
        default: {
            //! Do nothing    
        }
    }

    return CONTINUE_CAPTURING;
}

bool FrameCapturer::parseCapturedFrameOnL1(
    const ConstRawFrameViewType rawFrameBuffer, 
    const FrameFilter& filter,
    CallbackType callback 
) {
    const posnet::EthernetViewer ethernetViewer{ rawFrameBuffer };
    if (!filter.filter(ethernetViewer)) {
        return parseCapturedFrameOnL2(ethernetViewer, filter, callback);
    }

    return CONTINUE_CAPTURING;
}

bool FrameCapturer::parseCapturedFrame(
    const ConstRawFrameViewType rawFrameBuffer, 
    const FrameFilter& filters,
    CallbackType callback
) {
    return parseCapturedFrameOnL1(rawFrameBuffer, filters, callback);
}

void FrameCapturer::capture(const FrameFilter& filter, CallbackType callback)
{
    const auto sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    ASSERTION(sockfd > 0, BadFrameCapturer, utils::GetLastSysError())

    utils::ScopedLock lock([sockfd]{ close(sockfd); });
    std::array<def::ByteType, 65536> buffer = {0};
    
    while (true) {
        const ConstRawFrameViewType::size_type bufferSize = recv(sockfd, buffer.data(), buffer.size(), 0);
        ASSERTION(bufferSize > 0, BadFrameCapturer, utils::GetLastSysError());
        
        if (parseCapturedFrame(ConstRawFrameViewType{ buffer.data(), bufferSize }, filter, callback) == CONTINUE_CAPTURING) {
            //! Do nothing
        } else {
            break;
        }
    }   
}



} //! namespace posnet
