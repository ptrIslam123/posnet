#include "include/utils/algorithms.h"

#include "include/utils/sock_addr_convertor.h"
#include "include/utils/assert.h"

#include <sstream>
#include <iomanip>

namespace
{

} //! namespace


namespace posnet::utils {

void DumpToHexFormat(std::ostream& os, std::span<std::uint8_t> data)
{
    DumpToHexFormat(os, std::span<const std::uint8_t>(data));
}

void DumpToHexFormat(std::ostream& os, const std::span<const std::uint8_t> data)
{
    std::ios_base::fmtflags f(os.flags());
    os << std::hex << std::setfill('0');
    for (size_t i = 0; i < data.size(); ++i) {
        os << std::setw(2) << static_cast<unsigned>(data[i]) << " ";
    }
    os << std::endl;
    os.flags(f);
}

std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::set<std::string> GenerateIpAddrRange(const std::string_view ipAddr, int cidr)
{
    auto intIp = v4::StrToIpAddr(ipAddr);
    ASSERTION(intIp, std::runtime_error, "Invalid ip addr string format")

    uint32_t mask = ~((1 << (32 - cidr)) - 1);
    uint32_t network = *intIp & mask;
    uint32_t broadcast = network | ~mask;

    std::set<std::string> range;
    for (uint32_t i = network + 1; i < broadcast; ++i) {
        range.insert(v4::IpAddrToStr(i));
    }

    return range;
}

std::set<std::string> GenerateIpAddrRange(const std::string_view startIpAddr, const std::string_view endIpAddr)
{
    std::set<std::string> ipList;
    const auto startAddr = posnet::utils::v4::StrToIpAddrArray(startIpAddr);
    const auto endAddr = posnet::utils::v4::StrToIpAddrArray(endIpAddr);
    if (!startAddr || !endAddr) {
        return {};
    }

    for (auto i = 0; i < startAddr->size(); ++i) {
        const auto start = static_cast<uint32_t>(startAddr->at(i)); 
        const auto end = static_cast<uint32_t>(endAddr->at(i));

        if (start < end) {
            auto address = *startAddr;
            for (auto j = start; j <= end || j <= 254; ++j) {
                address[i] = j;
                auto addressSr = posnet::utils::v4::IpAddrToStr(address);
                if (!addressSr.empty()) {
                    ipList.insert(std::move(addressSr));
                }
            }
        }
    }

    ipList.insert(std::string(endIpAddr));
    return ipList;
}

std::uint16_t CalcChecksum(std::span<const std::uint8_t> packet)
{
    uint32_t sum = 0;  // Используем uint32_t для хранения суммы, чтобы избежать переполнения

    // Суммируем все 16-битные слова
    for (auto i = 0; i + 1 < packet.size(); i += 2) {
        sum += (packet[i] << 8) | packet[i + 1];
    }

    // Если есть нечетный байт, добавляем его в сумму
    if (packet.size() % 2 == 1) {
        sum += packet[packet.size() - 1] << 8;
    }

    // Складываем старшие биты (если есть перенос) с младшими
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Дополнение до единицы
    sum = ~sum;

    // Возвращаем 16-битный результат
    return static_cast<uint16_t>(sum & 0xFFFF);
}

std::uint16_t CalcChecksum(const std::span<std::uint8_t> packet) 
{
    std::size_t i;
    std::uint64_t sum = 0;
    auto buffer = packet.data();
    const auto size = packet.size();
    for (i = 0; i < size; i += 2) {
        sum += *(uint16_t *)buffer;
        buffer += 2;
    }

    if (size - i > 0) {
        sum += *(uint8_t *)buffer;
    }

    while ((sum >> 16) != 0) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

void HostBufferViewToNetwork(std::span<std::int8_t> buffer)
{
    //! TODO:
}

void HostBufferViewToNetwork(std::span<std::uint8_t> buffer)
{
    //! TODO:
}

} //! namespace posnet::utils
