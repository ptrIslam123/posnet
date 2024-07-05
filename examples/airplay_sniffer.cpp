#include <iostream>
#include <ostream>
#include <span>
#include <array>
#include <unordered_set>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cassert>

#include "include/frame-viewers/tcp_viewer.h"
#include "include/frame-capturer/frame_capturer.h"
#include "include/frame-filter/frame_filter.h"

#include "include/utils/algorithms.h"
#include "include/utils/scoped_lock.h"
#include "include/utils/assert.h"

#include <plist/plist.h>

constexpr auto DEFAULT_RAOP_PORT = 12345;

posnet::FrameFilter GetFilter() {
    posnet::FrameFilter filter;
    // Parse transport port
    {
        filter.srcPort.insert(DEFAULT_RAOP_PORT);
        filter.dstPort.insert(DEFAULT_RAOP_PORT);
    }

    // Parse ip addr
    {
        if (filter.srcIpAddr.empty()) {
            filter.srcIpAddr.insert(std::string{ posnet::FrameFilter::ANY_IP_ADDR });
        }

        if (filter.dstIpAddr.empty()) {
            filter.dstIpAddr.insert(std::string{ posnet::FrameFilter::ANY_IP_ADDR });
        }
    }

    // Parse eth addr
    {
        if (filter.srcHrwAddr.empty()) {
            filter.srcHrwAddr.insert(std::string{ posnet::FrameFilter::ANY_ETH_ADDR });
        }

        if (filter.dstHrwAddr.empty()) {
            filter.dstHrwAddr.insert(std::string{ posnet::FrameFilter::ANY_ETH_ADDR });
        }
    }

    // Parse protocols
    {
        const auto anyProtocols = posnet::FrameFilter::GetSupportedProtocols();
        for (const auto& protocol : anyProtocols) {
            filter.protocols.insert(std::string{ protocol });
        }
    }
    return filter;
}

std::string ExtractPlistXml(const std::string_view plistContent)
{
    plist_t rootNode = nullptr;
    plist_from_bin(plistContent.data(), plistContent.size(), &rootNode);

    char* xmlStr = nullptr;
    uint32_t xmlStrLen = 0;
    plist_to_xml(rootNode, &xmlStr, &xmlStrLen);
    std::string xmlContent{ xmlStr, xmlStrLen };
    free(xmlStr);
    return xmlContent;
}

void OnCapturedFrame(
    std::ostream& os, 
    const posnet::BaseFrame& baseFrame, 
    const posnet::FrameFilter::ProtocolType protocol,
    const int level
) {
    ASSERTION(protocol == posnet::FrameFilter::ProtocolType::TCP, std::runtime_error, "Bad protocol(not tcp)");
    posnet::utils::ScopedLock lock([&os] { os.flush(); });

    const posnet::IpViewer ipViewer{ baseFrame.getAsRawFrameView() };
    const posnet::TcpViewer tcpViewer{ baseFrame.getAsRawFrameView() };

    const auto srcIp = ipViewer.getSourceIpAddressAsStr();
    const auto dstIp = ipViewer.getDestIpAddressAsStr();
    const auto payload = tcpViewer.getPayload();
    const auto payloadSize = payload.size();
    if (payload.empty()) {
        return;
    }

    const std::string_view header{ (char*)payload.data(), payload.size() };

    os << "\n{\n";
    os << "src-ip-addr=" << srcIp << " : dst-ip-addr=" << dstIp << "\n\n";

    auto pos = header.find("bplist");
    if (pos != std::string_view::npos) {
        const std::string_view bplistBinaryContent{ header.data() + pos, header.size() - pos };
        const auto bplistXmlContent = ExtractPlistXml(bplistBinaryContent);
        os << std::string_view{ header.data(), header.size() - bplistBinaryContent.size() } << "\n\n";
        os << bplistXmlContent << "\n\n";
        posnet::utils::DumpToHexFormat(os, std::span<std::uint8_t>{ (std::uint8_t*)bplistBinaryContent.data(), bplistBinaryContent.size() });
        os << "\n\n}\n";
        return;
    }

    pos = header.find("FPLY");
    if (pos != std::string_view::npos) {
        std::array<posnet::def::ByteType, 2024> fplyBuffer = {0};

        const auto size = std::string_view{"FPLY"}.size();
        const auto fplyBinaryContentStart = payload.begin() + pos + size;
        const auto fplyBinaryContentEnd = payload.end();
        const auto fplyBinaryContentSize = fplyBinaryContentEnd - fplyBinaryContentStart;

        std::memcpy(fplyBuffer.data(), payload.data() + pos + size, fplyBinaryContentSize);

        const std::string_view fplyHeader{ header.data() + pos, size };

        os << fplyHeader << "\n\n";
        posnet::utils::DumpToHexFormat(os, std::span<std::uint8_t>{ fplyBuffer.data(), (std::span<std::uint8_t>::size_type)fplyBinaryContentSize });
        os << "\n}\n";
        return;
    }

    os << header << "\n\n";
    posnet::utils::DumpToHexFormat(os, std::span<std::uint8_t>{ (std::uint8_t*)header.data(), header.size() });
    os << "\n}\n";
}

int main(int argc, char** argv) {
    try {
        const auto filter = GetFilter();
        std::cout << filter << std::endl;

        posnet::FrameCapturer capturer;
        capturer.capture(filter, [&os = std::cout](
            const posnet::BaseFrame& baseFrame, 
            const posnet::FrameFilter::ProtocolType protocol,
            const int level
            ) {
            OnCapturedFrame(os, baseFrame, protocol, level);
            return posnet::FrameCapturer::CONTINUE_CAPTURING;
        });
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Throw unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}
