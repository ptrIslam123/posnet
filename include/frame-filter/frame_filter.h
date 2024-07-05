#ifndef VS_FRAME_FILTER_H
#define VS_FRAME_FILTER_H

#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>

#include "include/frame-viewers/ethernet_viewer.h"
#include "include/frame-viewers/arp_viewer.h"
#include "include/frame-viewers/ip_viewer.h"
#include "include/frame-viewers/tcp_viewer.h"
#include "include/frame-viewers/udp_viewer.h"
#include "include/frame-viewers/icmp_viewer.h"

namespace posnet {

class FrameFilter final {
public:
    static constexpr int ANY_PORT = 0;
    static constexpr std::string_view ANY_IP_ADDR = "any";
    static constexpr std::string_view ANY_ETH_ADDR = "any";
    static constexpr std::string_view ANY_PROTOCOL = "any";

    enum class ProtocolType {
        TCP, UDP, ICMP, // level 3
        IP, ARP, RARP,  // level 2
        ETHERNET,       // level 1
    };

    std::unordered_set<int> srcPort;
    std::unordered_set<int> dstPort;
    std::unordered_set<std::string> srcIpAddr;
    std::unordered_set<std::string> dstIpAddr;
    std::unordered_set<std::string> srcHrwAddr;
    std::unordered_set<std::string> dstHrwAddr;
    std::unordered_set<std::string> protocols;

    bool filter(const EthernetViewer& ethernet) const;
    bool filter(const ArpViewer& arp) const;
    bool filter(const IpViewer& ip) const;
    bool filter(const TcpViewer& tcp) const;
    bool filter(const UdpViewer& udp) const;
    bool filter(const IcmpViewer& icmp) const;

    static std::unordered_set<std::string_view> GetSupportedProtocols();
    static const std::unordered_set<std::string_view>& GetSupportedL3Protocols();
    static const std::unordered_set<std::string_view>& GetSupportedL2Protocols();
    static const std::unordered_set<std::string_view>& GetSupportedL1Protocols();

    std::ostream& operator<<(std::ostream& os) const;

private:
    bool filterWithEthAddr(std::string_view protocol, std::string_view srcAddr, std::string_view dstAddr) const;
    bool filterWithIpAddr(std::string_view protocol, std::string_view srcAddr, std::string_view dstAddr) const;
    bool filterWithPort(std::string_view protocol, unsigned short srcAddr, unsigned short dstAddr) const;
};

std::ostream& operator<<(std::ostream& os, const FrameFilter& filter);

} //! namespace posnet

#endif //! VS_FRAME_FILTER_H
