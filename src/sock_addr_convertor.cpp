#include "utils/sock_addr_convertor.h"

#include "utils/scoped_lock.h"

#include <sstream>
#include <iomanip>
#include <limits>
#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

#include <netdb.h>
#include <arpa/inet.h>

namespace {

std::optional<std::string> ValidateMacAddr(std::string_view macAddr)
{
    std::array<char, 32> addrStorage = {0};
    auto i = 0;
    do {
        const auto pos = macAddr.find(":");
        if (pos != std::string_view::npos) {
            const std::string_view actet = macAddr.substr(0, pos);
            if (actet.size() == 2) {
                std::memcpy(addrStorage.data() + i, actet.data(), actet.size());
                i += actet.size();

                addrStorage[i] = ':';
                i += 1;

            } else if (actet.size() == 1) {
                char zero = '0';
                std::memcpy(addrStorage.data() + i, &zero, sizeof(zero));
                i += sizeof(zero);

                std::memcpy(addrStorage.data() + i, actet.data(), actet.size());
                i += 1;

                addrStorage[i] = ':';
                i += 1;
            } else {
                return {};
            }
            macAddr = macAddr.substr(pos + 1);
        } else {
            std::memcpy(addrStorage.data() + i, macAddr.data(), macAddr.size());
            i += macAddr.size();
            break;
        }
    } while (true);

    return (i == macAddr.size() ?
                std::nullopt :
                std::make_optional(std::string{ addrStorage.data() })
    );
}

} //! namespace

namespace posnet::utils {

namespace v4 {

std::string IpAddrToStr(const std::span<uint8_t, IPV4_ADDRESS_LENGTH_IN_BYTES> ipAddr)
{
    static std::array<char, INET_ADDRSTRLEN> storage = {0};
    std::memset(storage.data(), 0, storage.size());

    inet_ntop(AF_INET, ipAddr.data(), storage.data(), INET_ADDRSTRLEN);
    return std::string(storage.data(), storage.size());
}

std::string IpAddrToStr(uint32_t ipAddr)
{
    auto addrStruct = reinterpret_cast<struct in_addr*>(&ipAddr);
    return inet_ntoa(*addrStruct);
}

std::optional<uint32_t> StrToIpAddr(const std::string_view ipAddrStr)
{
    struct in_addr addr;
    if (inet_aton(ipAddrStr.data(), &addr) != 0) {
        return addr.s_addr;
    } else {
        return std::nullopt;
    }
}

std::optional<std::array<std::uint8_t, IPV4_ADDRESS_LENGTH_IN_BYTES>> StrToIpAddrArray(const std::string_view ipAddrStr)
{
    std::array<std::uint8_t, IPV4_ADDRESS_LENGTH_IN_BYTES> ipAddr = {0};
    std::istringstream iss(ipAddrStr.data());
    std::string segment;
    auto i = 0;

    while (std::getline(iss, segment, '.')) {
        if (i >= 4 || segment.empty()) {
            return std::nullopt; // Invalid IP address format
        }

        std::istringstream converter(segment);
        int value;
        converter >> value;

        if (converter.fail() || value < 0 || value > 255) {
            return std::nullopt; // Invalid IP address value
        }

        ipAddr[i++] = static_cast<std::uint8_t>(value);
    }

    if (i != 4) {
        return std::nullopt; // Not enough segments
    }

    return ipAddr;
}

std::optional<uint32_t> CountSetBitsInIpAddr(const std::string_view ipAddrStr)
{
    const auto address = StrToIpAddr(ipAddrStr);
    if (address) {
        auto n = *address;
        uint32_t count = 0;
        while (n) {
            count += n & 1;
            n >>= 1;
        }

        return count;
    } else {
        return std::nullopt;
    }
}

} //! namespace v4


namespace v6 {

std::string IpAddrToStr(const struct sockaddr_in6& ipAddr)
{
    std::array<char, INET6_ADDRSTRLEN> str = {0};
    inet_ntop(AF_INET6, &(ipAddr.sin6_addr), str.data(), str.size());
    return std::string{ str.data() };
}

} //! namespace v6

std::string MacAddrToStr(std::span<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES> macAddr)
{
    std::array<char, 2 * 6 + 1 * 5> addrStorage = {0};
    auto addrStruct = reinterpret_cast<struct ether_addr*>(macAddr.data());
    const std::string_view addr{ ether_ntoa(addrStruct) };
    const auto res = ValidateMacAddr(addr);
    return (res.has_value() ? *res : std::string{ addr });
}

std::string MacAddrToStr(const struct sockaddr& macAddr)
{
    const auto addr = ether_ntoa(reinterpret_cast<const struct ether_addr*>(&macAddr.sa_data));
    if (addr) {
        const std::string addrStr{ addr };
        const auto res = ValidateMacAddr(addrStr);
        return res.has_value() ? *res : std::string{ addrStr };
    } else {
        return {};
    }
}

std::optional<std::array<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES>> StrToMacAddr(const std::string_view macAddrStr)
{
    std::array<int, MAC_ADDRESS_LENGTH_IN_BYTES> values = {0};
    std::array<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES> result = {0};
    int count = sscanf(macAddrStr.data(), "%x:%x:%x:%x:%x:%x", 
            &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]);
    if (count != 6) {
        return std::nullopt;
    }

    for (auto i = 0; i < 6; ++i) {
        result[i] = static_cast<uint8_t>(values[i]);
    }
    return result;
}

} //! namespace posnet::utils
