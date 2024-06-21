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

    return !(srcAttrSatisfaction || dstAttrSatisfaction);
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
    const auto satisfyAnyProtocols = (protocols.size() == 1 && protocols.find("any") != protocols.cend());
    const auto satisfyProtocol = (satisfyAnyProtocols ? true :  protocols.find(protocol.data()) != protocols.cend());

    const auto satisfyAnySrc = (srcIpAddr.size() == 1 && srcIpAddr.find("any") != srcIpAddr.cend());
    const auto satisfyAnyDst = (dstIpAddr.size() == 1 && dstIpAddr.find("any") != dstIpAddr.cend());

    const auto addrPresentedInSrcSet = srcIpAddr.find(srcAddr.data()) != srcIpAddr.cend();
    const auto addrPresentedInDstSet = dstIpAddr.find(dstAddr.data()) != dstIpAddr.cend();

    const auto srcAttrSatisfaction = (satisfyAnySrc || addrPresentedInSrcSet);
    const auto dstAttrSatisfaction = (satisfyAnyDst || addrPresentedInDstSet);

    return !(satisfyProtocol && (srcAttrSatisfaction || dstAttrSatisfaction));
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
    const auto satisfyAnyProtocols = (protocols.size() == 1 && protocols.find("any") != protocols.cend());
    const auto satisfyProtocol = (satisfyAnyProtocols ? true :  protocols.find(protocol.data()) != protocols.cend());
    const auto satisfyAnySrc = (srcPort.size() == 1 && srcPort.find(0) != srcPort.cend());
    const auto satisfyAnyDst = (dstPort.size() == 1 && dstPort.find(0) != dstPort.cend());


    const auto addrPresentedInSrcSet = srcPort.find(srcAddr) != srcPort.cend();
    const auto addrPresentedInDstSet = dstPort.find(dstAddr) != dstPort.cend();

    const auto srcAttrSatisfaction = (satisfyAnySrc || addrPresentedInSrcSet);
    const auto dstAttrSatisfaction = (satisfyAnyDst || addrPresentedInDstSet);

    return !(satisfyProtocol && (srcAttrSatisfaction || dstAttrSatisfaction));
}

bool FrameFilter::filter(const IcmpViewer& icmp) const
{
    return true;
}

} //! namespace posnet
