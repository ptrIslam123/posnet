#include "frame-builder/udp_builder.h"

#include "frame-viewers/ip_viewer.h"
#include "frame-viewers/ethernet_viewer.h"
#include "utils/algorithms.h"

#include <sstream>
#include <cstring>

#include <netinet/in.h>

namespace posnet {
    
UdpBuilder::UdpBuilder(const RawFrameViewType rawFrame):
m_frame(reinterpret_cast<HeaderStructType*>(rawFrame.data() + sizeof(EthernetViewer::HeaderStructType) + IpViewer(rawFrame).getHeaderLengthInBytes())),
m_rawFrame(rawFrame),
m_payloadSize(0)
{}

UdpBuilder& UdpBuilder::setSourcePort(const PortType port) &
{
    m_frame->dest = ntohs(port);
    return *this;
}

UdpBuilder& UdpBuilder::setDestPort(const PortType port) &
{
    m_frame->source = ntohs(port);
    return *this;
}

UdpBuilder& UdpBuilder::setUdpDataGramLength(const unsigned int length) &
{
    m_frame->len = ntohs(length);
    return *this;
}

UdpBuilder& UdpBuilder::setCheckSum(const unsigned int checkSum) &
{
    m_frame->check = ntohs(checkSum);
    return *this;
}

UdpBuilder& UdpBuilder::setPayload(const std::span<std::int8_t> payload) &
{
    if (sizeof(EthernetViewer::HeaderStructType) + IpViewer(m_rawFrame).getHeaderLengthInBytes() + payload.size() <= m_rawFrame.size()) {
        std::memcpy(m_frame + sizeof(HeaderStructType), payload.data(), payload.size());    
    } else {
        std::stringstream ss;
        ss << "Attempt to write payload that is bigger than frame buffer. Payload size=" 
            << payload.size() << ", frame buffer size=" << m_rawFrame.size();
        throw std::runtime_error(ss.str());
    }
    
    return *this;
}

UdpBuilder& UdpBuilder::setPayload(const std::span<std::uint8_t> payload) &
{
    const auto totalLength = payload.size() + EthernetViewer::DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES + 
        IpViewer(m_rawFrame).getHeaderLengthInBytes() + DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES;
    
    if (totalLength < m_rawFrame.size()) {
        std::memcpy(m_frame + DEFAULT_FRAME_HEADER_LENGTH_IN_BYTES, payload.data(), payload.size());
        m_payloadSize = payload.size();
    } else {
        throw std::runtime_error("Attempt to write payload that is bigger than initial package buffer");
    }

    return *this;
}

unsigned int UdpBuilder::getDefaultCheckSum()
{
    /*
    Да, вы правы. При вычислении контрольной суммы UDP-пакета, которая должна быть включена в заголовок UDP, 
    вы должны включить в сумму все 16-битные слова от начала IP-заголовка до конца данных UDP-пакета. 
    Это необходимо для того, чтобы обеспечить целостность данных, передаваемых по сети.
    Контрольная сумма вычисляется для целого пакета, включая IP-заголовок и данные UDP-пакета. 
    Если бы вы вычислили контрольную сумму только для данных UDP, то при передаче пакета через сеть могли 
    бы возникнуть проблемы с целостностью данных, если они по какой-либо причине были изменены на пути.
    Таким образом, для вычисления контрольной суммы UDP-пакета, которая должна быть включена в заголовок UDP, 
    необходимо учитывать IP-заголовок и данные UDP-пакета. Это позволяет обнаруживать изменения в любой части пакета, 
    включая IP-заголовок, который также содержит информацию о проверке целостности.
    */
    return posnet::utils::CalcChecksum(ConstRawFrameViewType{ 
        reinterpret_cast<const std::uint8_t*>(m_rawFrame.data() + sizeof(EthernetViewer::HeaderStructType)),
        IpViewer(m_rawFrame).getHeaderLengthInBytes() + sizeof(HeaderStructType) + m_payloadSize
    });
}

unsigned int UdpBuilder::getDefaultCheckSum() const
{
    return posnet::utils::CalcChecksum(ConstRawFrameViewType{ 
        reinterpret_cast<const std::uint8_t*>(m_rawFrame.data() + sizeof(EthernetViewer::HeaderStructType)),
        IpViewer(m_rawFrame).getHeaderLengthInBytes() + sizeof(HeaderStructType) + m_payloadSize
    });
}

} // namespace posnet