#include "ip_builder.h"

#include "src/frame-viewers/ethernet_viewer.h"
#include "src/utils/sock_addr_convertor.h"
#include "src/utils/algorithms.h"

#include <stdexcept>
#include <sstream>

namespace posnet {
    
IpBuilder::IpBuilder(const RawFrameVieType rawFrame):
m_frame(reinterpret_cast<HeaderStructType*>(rawFrame.data() + sizeof(EthernetViewer::HeaderStructType))),
m_rawFrame(rawFrame)
{}

IpBuilder& IpBuilder::setVersion(const VersionType version) &
{
    switch (version) {
        case VersionType::V4: {
            m_frame->version = 4;
            break;
        }
        case VersionType::V6: {
            m_frame->version = 6;
            break;
        }
        default:
            throw std::runtime_error("Undefined version of ip frame=" + 
                std::to_string(static_cast<unsigned int>(version)));
    }

    return *this;
}

IpBuilder& IpBuilder::setProtocol(const ProtocolType protocol) &
{
    switch (protocol) {
        case ProtocolType::TCP: {
            m_frame->protocol = IPPROTO_TCP;
            break;
        }
        case ProtocolType::UDP: {
            m_frame->protocol = IPPROTO_UDP;
            break;
        }
        case ProtocolType::ICMP: {
            m_frame->protocol = IPPROTO_ICMP;
            break;
        }
        default:
            throw std::runtime_error("Undefined protocol type of ip frame=" + 
                std::to_string(static_cast<unsigned int>(protocol)));
    }

    return *this;
}

IpBuilder& IpBuilder::setTypeOfService(const unsigned int tos) &
{
    m_frame->tos = tos;
    return *this;
}

IpBuilder& IpBuilder::setHeaderLengthInBytes(const unsigned int length) &
{
    m_frame->ihl = length / 4;
    return *this;
}

IpBuilder& IpBuilder::setTotalLength(const unsigned int length) &
{
    /*
    Поле tot_len (Total Length) в IP-заголовке указывает на общую длину пакета в байтах, включая заголовок и данные. 
    Оно хранит длину в 16-битном формате, поэтому его значение должно быть представлено в сетевом порядке байтов (big-endian).  
    Чтобы вычислить tot_len, вы должны сложить длину заголовка IP (который обычно составляет 20 байт без опций) и длину данных, 
    которые следуют за IP-заголовком. Длина данных указывается в октетах, поэтому вам нужно умножить ее на 4, чтобы получить длину в байтах.
    */
    m_frame->tot_len = htons(length);
    return *this;
}

IpBuilder& IpBuilder::setId(const unsigned int id) &
{
    m_frame->id = htons(id);
    return *this;
}

IpBuilder& IpBuilder::setFragmentOffset(const bool needToFragmentPackage, const unsigned int offset) &
{
    /*
    Когда вы создаете пакеты IP вручную, вы, как правило, не выполняете фрагментацию вручную. 
    Фрагментация обычно выполняется автоматически при передаче данных через сетевые интерфейсы или стеки протоколов, такие как TCP/IP.
    Когда данные передаются по сети, узел-отправитель (например, ваш компьютер) фрагментирует пакеты IP, если они превышают максимальный размер, 
    который может передаваться без фрагментации на любом сетевом пути между отправителем и получателем. 
    Каждый фрагмент получает свой собственный заголовок IP с соответствующим смещением фрагмента.
    Когда пакет достигает узла-получателя, он (или компоненты сети, такие как маршрутизаторы или шлюзы) перестраивают 
    пакеты обратно в исходный пакет IP, используя смещение фрагмента для правильного упорядочения фрагментов.
    Таким образом, обычно вам не нужно беспокоиться о фрагментации и смещениях фрагментов при создании пакетов 
    IP вручную, если вы используете протоколы, которые управляют этим процессом, такие как TCP. Ваша задача - установить необходимые поля IP-заголовка,
    включая адреса источника и назначения, и передать данные через соответствующий интерфейс. Остальная работа будет выполнена протоколами нижнего уровня.
    Если вы хотите создать пакеты IP вручную для тестирования или для реализации нестандартного поведения, вы можете вручную установить 
    смещение фрагмента, но это обычно не требуется для обычных сценариев использования. Ваша задача - убедиться, что вы правильно устанавливаете 
    другие поля IP-заголовка, такие как идентификатор, флаги фрагментации, время жизни (TTL) и т. д., чтобы пакеты правильно маршрутизировались и обработались.
    */
    if (offset > MAX_FRAME_FRAGMENT_OFFSET_VALUE) {
        throw std::runtime_error("");
    }

    uint16_t value = offset & 0x1FFF; // set offset value and clear hight 3 bits for flag
    if (needToFragmentPackage) {
        value |= 0x2000; // set "More Fragments"(MF) flag value
    } else {
        value |= 0x4000; // set "Don't Fragment"(DF) flag value
    }

    m_frame->frag_off = htons(value);
    return *this;
}

IpBuilder& IpBuilder::setTTL(const unsigned int ttl) &
{
    m_frame->ttl =  ttl;
    return *this;
}

IpBuilder& IpBuilder::setCheckSum(const unsigned int checkSum) &
{
    m_frame->check = htons(checkSum);
    return *this;
}

IpBuilder& IpBuilder::setSourceIpAddress(const std::string_view ipAddr) &
{
    const auto result = posnet::utils::StrToIpAddr(ipAddr);
    if (result) {
        m_frame->saddr = *result;
    } else {
        std::stringstream ss;
        ss << "Could not set source ip-address for=" << ipAddr << ". Invalid ip-address";
        throw std::runtime_error(ss.str());
    }
    return *this;
}

IpBuilder& IpBuilder::setDestIpAddress(const std::string_view ipAddr) &
{
    const auto result = posnet::utils::StrToIpAddr(ipAddr);
    if (result) {
        m_frame->daddr = *result;
    } else {
         std::stringstream ss;
        ss << "Could not set destination ip-address for=" << ipAddr << ". Invalid ip-address";
        throw std::runtime_error(ss.str());
    }
    return *this;
}

unsigned int IpBuilder::getDefaultCheckSum()
{
    /*
    Для вычисления контрольной суммы IP-заголовка, которая должна быть включена в IP-заголовок, 
    вы должны включить в сумму все 16-битные слова от начала IP-заголовка до конца IP-заголовка. 
    Это означает, что вы должны включить в сумму все поля IP-заголовка, но не включать в сумму данные, которые следуют за IP-заголовком.
    Вот как вы можете определить границы для вычисления контрольной суммы IP-заголовка:
        Начните с начала IP-заголовка.
        Закончите перед полем контрольной суммы в IP-заголовке.
    */
    return posnet::utils::CalcChecksum(
        ConstRawVieType{ reinterpret_cast<const std::uint8_t*>(m_rawFrame.data() + sizeof(EthernetViewer::HeaderStructType)), sizeof(HeaderStructType) }
    );
}

unsigned int IpBuilder::getDefaultCheckSum() const
{
    return posnet::utils::CalcChecksum(
        ConstRawVieType{ reinterpret_cast<const std::uint8_t*>(m_rawFrame.data() + sizeof(EthernetViewer::HeaderStructType)), sizeof(HeaderStructType) }
    );
}

} //! namespace posnet