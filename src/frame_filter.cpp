#include "include/frame-filter/frame_filter.h"

#include <algorithm>
#include <cstring>

namespace posnet {

bool FrameFilter::filter(const EthernetViewer& ethernet) const
{   
    const auto srcAddr = ethernet.getSourceMacAddressAsStr();
    const auto dstAddr = ethernet.getDestMacAddressAsStr();

    const auto satisfyAnySrc = srcHrwAddr.size() == 1 && srcHrwAddr.find("any") != srcHrwAddr.cend();
    const auto satisfyAnyDst = dstHrwAddr.size() == 1 && dstHrwAddr.find("any") != dstHrwAddr.cend();

    const auto addrPresentedInSrcSet = srcHrwAddr.find(srcAddr) != srcHrwAddr.cend();
    const auto addrPresentedInDstSet = dstHrwAddr.find(dstAddr) != dstHrwAddr.cend();

    const auto srcAttrSatisfaction = (satisfyAnySrc || addrPresentedInSrcSet);
    const auto dstAttrSatisfaction = (satisfyAnyDst || addrPresentedInDstSet);

    const auto dropFrame = !(srcAttrSatisfaction || dstAttrSatisfaction);;
    return dropFrame;
}

bool FrameFilter::filterWithEthAddr(std::string_view protocol, std::string_view srcAddr, std::string_view dstAddr) const
{
    const auto satisfyAnySrc = srcHrwAddr.size() == 1 && srcHrwAddr.find("any") != srcHrwAddr.cend();
    const auto satisfyAnyDst = dstHrwAddr.size() == 1 && dstHrwAddr.find("any") != dstHrwAddr.cend();

    const auto addrPresentedInSrcSet = srcHrwAddr.find(srcAddr.data()) != srcHrwAddr.cend();
    const auto addrPresentedInDstSet = dstHrwAddr.find(dstAddr.data()) != dstHrwAddr.cend();

    const auto srcAttrSatisfaction = (satisfyAnySrc || addrPresentedInSrcSet);
    const auto dstAttrSatisfaction = (satisfyAnyDst || addrPresentedInDstSet);

    const auto dropFrame = !(srcAttrSatisfaction || dstAttrSatisfaction);;
    return dropFrame;
}

bool FrameFilter::filter(const ArpViewer& arp) const
{
    return true;
}

bool FrameFilter::filter(const IpViewer& ip) const
{
    return filterWithIpAddr("ip", ip.getSourceIpAddressAsStr(), ip.getDestIpAddressAsStr());
}

bool FrameFilter::filterWithIpAddr(
        const std::string_view protocol,
        const std::string_view srcAddr,
        const std::string_view dstAddr
) const
{
    const auto satisfyProtocol = (protocols.find(protocol.data()) != protocols.cend());
    const auto satisfyAnySrc = (srcIpAddr.size() == 1 && srcIpAddr.find("any") != srcIpAddr.cend());
    const auto satisfyAnyDst = (dstIpAddr.size() == 1 && dstIpAddr.find("any") != dstIpAddr.cend());

    const auto addrPresentedInSrcSet = srcIpAddr.find(srcAddr.data()) != srcIpAddr.cend();
    const auto addrPresentedInDstSet = dstIpAddr.find(dstAddr.data()) != dstIpAddr.cend();

    const auto srcAttrSatisfaction = (satisfyAnySrc || addrPresentedInSrcSet);
    const auto dstAttrSatisfaction = (satisfyAnyDst || addrPresentedInDstSet);

    const auto dropFrame = !(satisfyProtocol && (srcAttrSatisfaction || dstAttrSatisfaction));
    return dropFrame;
}

bool FrameFilter::filter(const TcpViewer& tcp) const
{
    return filterWithPort("tcp", tcp.getSourcePort(), tcp.getDestPort());
}

bool FrameFilter::filter(const UdpViewer& udp) const
{
    return filterWithPort("udp", udp.getSourcePort(), udp.getDestPort());
}

bool FrameFilter::filterWithPort(
        const std::string_view protocol,
        const unsigned short srcAddr,
        const unsigned short dstAddr
) const
{
    const auto satisfyProtocol = (protocols.find(protocol.data()) != protocols.cend());
    const auto satisfyAnySrc = (srcPort.size() == 1 && srcPort.find(0) != srcPort.cend());
    const auto satisfyAnyDst = (dstPort.size() == 1 && dstPort.find(0) != dstPort.cend());

    const auto addrPresentedInSrcSet = srcPort.find(srcAddr) != srcPort.cend();
    const auto addrPresentedInDstSet = dstPort.find(dstAddr) != dstPort.cend();

    const auto srcAttrSatisfaction = (satisfyAnySrc || addrPresentedInSrcSet);
    const auto dstAttrSatisfaction = (satisfyAnyDst || addrPresentedInDstSet);

    const auto dropFrame = !(satisfyProtocol && (srcAttrSatisfaction || dstAttrSatisfaction));
    return dropFrame;
}

bool FrameFilter::filter(const IcmpViewer& icmp) const
{
    const posnet::IpViewer ip{ icmp.getAsRawFrameView() };
    return filterWithIpAddr("icmp", ip.getSourceIpAddressAsStr(), ip.getDestIpAddressAsStr());
}

std::unordered_set<std::string_view> FrameFilter::GetSupportedProtocols()
{

    std::unordered_set<std::string_view> protocols;
    const auto& l1Protocols = GetSupportedL1Protocols();
    const auto& l2Protocols = GetSupportedL2Protocols();
    const auto& l3Protocols = GetSupportedL3Protocols();
    protocols.insert(l1Protocols.begin(), l1Protocols.end());
    protocols.insert(l2Protocols.begin(), l2Protocols.end());
    protocols.insert(l3Protocols.begin(), l3Protocols.end());
    return protocols;
}

const std::unordered_set<std::string_view>& FrameFilter::GetSupportedL3Protocols()
{
    static const std::unordered_set<std::string_view> protocols = {"tcp", "udp", "icmp"};
    return protocols;
}

const std::unordered_set<std::string_view>& FrameFilter::GetSupportedL2Protocols()
{
    static const std::unordered_set<std::string_view> protocols = {"ip", "eth"};
    return protocols;
}

const std::unordered_set<std::string_view>& FrameFilter::GetSupportedL1Protocols()
{
    static const std::unordered_set<std::string_view> protocols = {"eth", "arp", "rarp"};
    return protocols;
}


std::ostream& FrameFilter::operator<<(std::ostream& os) const
{
    os << "Frame Filter {" << "\n";
    
    os << "\n";
    
    for (const auto& src : srcPort) {
        os  << "\t" << "src-port=";
        if (src == 0) {
            os << "any";
        } else {
            os << src;
        }
        os << "\n";
    }

    for (const auto& dst : dstPort) {
        os  << "\t" << "dst-port=";
        if (dst == 0) {
            os << "any";
        } else {
            os << dst;
        }
        os << "\n";
    }

    os << "\n";

    for (const auto& src : srcIpAddr) {
        os << "\t" << "src-ip-addr=" << src << "\n";
    }

    for (const auto& dst : dstIpAddr) {
        os << "\t" << "dst-ip-addr=" << dst << "\n";
    }

    os << "\n";

    for (const auto& src : srcHrwAddr) {
        os << "\t" << "src-eth-addr=" << src << "\n"; 
    }

    for (const auto& dst : dstHrwAddr) {
        os << "\t" << "dst-eth-addr=" << dst << "\n"; 
    }

    os << "\n";

    for (const auto& protocol : protocols) {
        os << "\t" << "protocol=" << protocol << "\n";
    }

    os << "\n" << "}\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const FrameFilter& filter)
{
    return filter.operator<<(os);
}

} //! namespace posnet
