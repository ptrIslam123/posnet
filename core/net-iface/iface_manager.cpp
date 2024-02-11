#include "iface_manager.h"

#include "utils/system_error.h"
#include "utils/sock_addr_convertor.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <span>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace posix::error;

namespace net::posix::low_level {

IFaceManager::IFaceManager():
m_socket(0)
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        throw std::runtime_error("Could not open socket: " + GetLastSysError());
    }
}

IFaceManager::~IFaceManager()
{
    close(m_socket);
    m_socket = 0;
}

void IFaceManager::setConfig(const Configuration& config)
{
    assert(m_socket > 0);
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));

    // Set Name
    if (!config.getName()) {
        throw std::runtime_error("");
    }
    strncpy(ifr.ifr_name, config.getName()->data() ,IFNAMSIZ);

    // Set Ip address
    if (config.getIpAddress()) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_addr;
        sin->sin_family = AF_INET;
        inet_pton(AF_INET, config.getIpAddress()->data(), &sin->sin_addr);
        if (ioctl(m_socket, SIOCSIFADDR, &ifr) < 0) {
            throw std::runtime_error("Could not set ip address: " + GetLastSysError());
        }
    }

    // Set Mac address
    if (config.getMacAddress()) {
        auto macAddr = utils::net::posix::low_level::StrToMacAddr(*config.getMacAddress());
        if (macAddr) {
            std::memcpy(ifr.ifr_hwaddr.sa_data, macAddr->data(), 6);
        } else {
            throw std::runtime_error("Could not set mac addr: invalid mac address format(expected forma: XX:XX:XX:XX:XX)");
        }
        
        if (ioctl(m_socket, SIOCSIFHWADDR, &ifr) < 0) {
            throw std::runtime_error("Could not set mac address: " + GetLastSysError());
        }
    }

    // Set Net mask address
    if (config.getNetMaskAddress()) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_netmask;
        sin->sin_family = AF_INET;
        inet_pton(AF_INET, config.getMacAddress()->data(), &sin->sin_addr);

        if (ioctl(m_socket, SIOCSIFNETMASK, &ifr) < 0) {
            throw std::runtime_error("Could not set net mask: " + GetLastSysError());
        }
    }

    // Set Broadcast address
    if (config.getBroadcastAddress()) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_broadaddr;
        sin->sin_family = AF_INET;
        inet_pton(AF_INET, config.getBroadcastAddress()->data(), &sin->sin_addr);
        if (ioctl(m_socket, SIOCSIFBRDADDR, &ifr) < 0) {
            throw std::runtime_error("Could not set broadcast address: " + GetLastSysError());
        }
    }

    // Set MTU
    if (config.getMTU()) {
        ifr.ifr_ifru.ifru_mtu = *config.getMTU();
        if (ioctl(m_socket, SIOCGIFMTU, &ifr) < 0) {
            throw std::runtime_error("Could not set MTU: " + GetLastSysError());
        }
    }

    // Set Status
    if (config.getStatus()) {
        switch (*config.getStatus()) {
            case Configuration::Status::Up: {
                ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
                break;
            }
            case Configuration::Status::Down: {
                ifr.ifr_flags |= IFF_NOARP;
                break;
            }
        }

        if (ioctl(m_socket, SIOCGIFFLAGS, &ifr) < 0) {
            throw std::runtime_error("Could not set status: " + GetLastSysError());
        }
    }
}

std::vector<IFaceManager::Configuration> IFaceManager::getConfigs()
{
    assert(m_socket > 0);
    std::array<char, 1024> buffer = {0};
    std::vector<Configuration> result;
    struct ifconf ifc;
    ifc.ifc_len = buffer.size();
    ifc.ifc_buf = buffer.data();

    if (ioctl(m_socket, SIOCGIFCONF, &ifc) < 0) {
        throw std::runtime_error("Could not get iface configuration: " + GetLastSysError());
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    for (; it != end; ++it) {
        Configuration conf;

        // Get Name
        conf.setName(it->ifr_name);

        // Get MAC address
        if (ioctl(m_socket, SIOCGIFHWADDR, it) == 0) {
            //std::span<uint8_t> addr(it->ifr_hwaddr.sa_data, 6); // it->ifr_hwaddr.sa_data
            conf.setMacAddress(
                        utils::net::posix::low_level::MacAddrToStr(it->ifr_hwaddr)
            );
        }

        // Get IP address
        if (ioctl(m_socket, SIOCGIFADDR, it) == 0) {
            auto sockAddr = (struct sockaddr_in*)(&(it->ifr_addr));
            conf.setIpAddress(inet_ntoa(sockAddr->sin_addr));
        }

        // Get Netmask
        if (ioctl(m_socket, SIOCGIFNETMASK, it) == 0) {
            auto sockAddr = (struct sockaddr_in*)(&(it->ifr_netmask));
            conf.setNetMaskAddress(inet_ntoa(sockAddr->sin_addr));
        }

        // Get Broadcast Address
        if (ioctl(m_socket, SIOCGIFBRDADDR, it) == 0) {
            auto sockAddr = (struct sockaddr_in*)(&(it->ifr_broadaddr));
            conf.setBroadcastAddress(inet_ntoa(sockAddr->sin_addr));
        }

        // Get MTU
        if (ioctl(m_socket, SIOCGIFMTU, it) == 0) {
            conf.setMTU(it->ifr_ifru.ifru_mtu);
        }

        // Get Index
        if (ioctl(m_socket, SIOCGIFINDEX, it) == 0) {
            conf.setIndex(it->ifr_ifru.ifru_ivalue);
        } 

        // Get Status
        if (ioctl(m_socket, SIOCGIFFLAGS, it) == 0) {
            const auto isUpIFace = ((it->ifr_flags & IFF_UP) != 0);
            conf.setStatus(isUpIFace ? Configuration::Status::Up : Configuration::Status::Down);
        }

        result.push_back(conf);
    }

    return result;
}

std::ostream& IFaceManager::Configuration::operator<<(std::ostream& os) const
{
    os << "Net-Iface conf{" << "\n";
    os << "\tname=" << (m_name ? *m_name : "None") << "\n";
    os << "\tstatus=";
    if (m_status) {
        switch (*m_status) {
            case Status::Up: {
                os << "UP";
                break;
            }
            case Status::Down: {
                os << "Down";
                break;
            }
        }
    } else {
        os << "None";
    }
    os << "\n";

    os << "\tmac-address=" << (m_macAddr ? *m_macAddr : "None") << "\n";

    os << "\tip-address=" << (m_ipAddr ? *m_ipAddr : "None") << "\n";

    os << "\tnet-mask-address=" << (m_netMaskAddr ? *m_netMaskAddr : "None") << "\n";

    os << "\tbroadcast-address=" << (m_broadcastAddr ? *m_broadcastAddr : "None") << "\n";

    os << "\tindex=";
    if (m_index) {
        os << *m_index;
    } else {
        os << "None";
    }
    os << "\n";

    os << "\tmtu=";
    if (m_mtu) {
        os << *m_mtu;
    } else {
        os << "None";
    }
    os << "\n";

    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const IFaceManager::Configuration& config)
{
    return config.operator<<(os);
}

std::optional<std::string_view> IFaceManager::Configuration::getName()
{
    return m_name;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getMacAddress()
{
    return m_macAddr;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getIpAddress()
{
    return m_ipAddr;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getNetMaskAddress()
{
    return m_netMaskAddr;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getBroadcastAddress()
{
    return m_broadcastAddr;
}

std::optional<IFaceManager::IndexType> IFaceManager::Configuration::getIndex()
{
    return m_index;
}

std::optional<IFaceManager::MTUType> IFaceManager::Configuration::getMTU()
{
    return m_mtu;
}

std::optional<IFaceManager::Configuration::Status> IFaceManager::Configuration::getStatus()
{
    return m_status;
}

std::optional<std::string_view> IFaceManager::Configuration::getName() const
{
    return m_name;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getMacAddress() const
{
    return m_macAddr;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getIpAddress() const
{
    return m_ipAddr;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getNetMaskAddress() const
{
    return m_netMaskAddr;
}

std::optional<IFaceManager::AddressType> IFaceManager::Configuration::getBroadcastAddress() const
{
    return m_broadcastAddr;
}

std::optional<IFaceManager::IndexType> IFaceManager::Configuration::getIndex() const
{
    return m_index;
}

std::optional<IFaceManager::MTUType> IFaceManager::Configuration::getMTU() const
{
    return m_mtu;
}

std::optional<IFaceManager::Configuration::Status> IFaceManager::Configuration::getStatus() const
{
    return m_status;
}

IFaceManager::Configuration& IFaceManager::Configuration::setName(const std::string_view name)
{
    m_name = std::string(name);
    return *this;
}

IFaceManager::Configuration& IFaceManager::Configuration::setMacAddress(const AddressType& addr)
{
    m_macAddr = addr;
    return *this;
}

IFaceManager::Configuration& IFaceManager::Configuration::setIpAddress(const AddressType& addr)
{
    m_ipAddr = addr;
    return *this;
}

IFaceManager::Configuration& IFaceManager::Configuration::setNetMaskAddress(const AddressType& addr)
{
    m_netMaskAddr = addr;
    return *this;
}

IFaceManager::Configuration& IFaceManager::Configuration::setBroadcastAddress(const AddressType& addr)
{
    m_broadcastAddr = addr;
    return *this;
}

IFaceManager::Configuration& IFaceManager::Configuration::setIndex(const IndexType index)
{
    if (!m_index) {
        m_index = index;
    } else {
        throw std::runtime_error("");
    }
    return *this;
}

IFaceManager::Configuration& IFaceManager::Configuration::setMTU(const MTUType mtu)
{
    m_mtu = mtu;
    return *this;
}

IFaceManager::Configuration& IFaceManager::Configuration::setStatus(const Status status)
{
    m_status = status;
    return *this;
}

} //! namespace net::posix::low_level
