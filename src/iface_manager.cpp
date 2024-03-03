#include "net-iface/iface_manager.h"

#include "utils/system_error.h"
#include "utils/sock_addr_convertor.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <span>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

using namespace posnet::utils;

namespace {

void EnablePromiscuousMode(const std::string_view ifaceName, const int socket)
{
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifaceName.data() ,IFNAMSIZ);

    // Get current iface flag value
    if (ioctl(socket, SIOCGIFFLAGS, &ifr) < 0) {
        throw std::runtime_error("");
    }

    // Enable the iface to promisc mode 
    ifr.ifr_flags |= IFF_PROMISC;

    // Set the new iface flag value with enabled promisc mode
    if (ioctl(socket, SIOCSIFFLAGS, &ifr) < 0) {
        throw std::runtime_error("");
    }
}


void DisablePromiscuousMode(const std::string_view ifaceName, const int socket)
{
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifaceName.data() ,IFNAMSIZ);     

    // Get current iface flag value
    if (ioctl(socket, SIOCGIFFLAGS, &ifr) < 0) {
        throw std::runtime_error("");
    }

    // Clear the IFF_PROMISC flag to disable promiscuous mode
    ifr.ifr_flags &= ~IFF_PROMISC;

    // Set the new flags
    if (ioctl(socket, SIOCSIFFLAGS, &ifr) < 0) {
        throw std::runtime_error("");
    }
}

std::vector<posnet::IFaceConfiguration> GetConfigs(const int socket)
{
    using Configuration = posnet::IFaceConfiguration;

    std::array<char, 1024> buffer = {0};
    std::vector<Configuration> result;
    struct ifconf ifc;
    ifc.ifc_len = buffer.size();
    ifc.ifc_buf = buffer.data();

    if (ioctl(socket, SIOCGIFCONF, &ifc) < 0) {
        throw std::runtime_error("Could not get iface configuration: " + GetLastSysError());
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    for (; it != end; ++it) {
        Configuration conf;

        // Get Name
        conf.setName(it->ifr_name);

        // Get MAC address
        if (ioctl(socket, SIOCGIFHWADDR, it) == 0) {
            conf.setMacAddress(
                        posnet::utils::MacAddrToStr(it->ifr_hwaddr)
            );
        }

        // Get IP address
        if (ioctl(socket, SIOCGIFADDR, it) == 0) {
            auto sockAddr = (struct sockaddr_in*)(&(it->ifr_addr));
            conf.setIpAddress(inet_ntoa(sockAddr->sin_addr));
        }

        // Get Netmask
        if (ioctl(socket, SIOCGIFNETMASK, it) == 0) {
            auto sockAddr = (struct sockaddr_in*)(&(it->ifr_netmask));
            conf.setNetMaskAddress(inet_ntoa(sockAddr->sin_addr));
        }

        // Get Broadcast Address
        if (ioctl(socket, SIOCGIFBRDADDR, it) == 0) {
            auto sockAddr = (struct sockaddr_in*)(&(it->ifr_broadaddr));
            conf.setBroadcastAddress(inet_ntoa(sockAddr->sin_addr));
        }

        // Get MTU
        if (ioctl(socket, SIOCGIFMTU, it) == 0) {
            conf.setMTU(it->ifr_ifru.ifru_mtu);
        }

        // Get Index
        if (ioctl(socket, SIOCGIFINDEX, it) == 0) {
            conf.setIndex(it->ifr_ifru.ifru_ivalue);
        } 

        // Get Status
        if (ioctl(socket, SIOCGIFFLAGS, it) == 0) {
            const auto isUpIFace = ((it->ifr_flags & IFF_UP) != 0);
            conf.setStatus(isUpIFace ? Configuration::Status::Up : Configuration::Status::Down);
        }

        result.push_back(conf);
    }

    return result;
}

void SetConfig(const int socket, const posnet::IFaceConfiguration& config)
{
    using Configuration = posnet::IFaceConfiguration;

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
        if (ioctl(socket, SIOCSIFADDR, &ifr) < 0) {
            throw std::runtime_error("Could not set ip address: " + GetLastSysError());
        }
    }

    // Set Mac address
    if (config.getMacAddress()) {
        auto macAddr = posnet::utils::StrToMacAddr(*config.getMacAddress());
        if (macAddr) {
            std::memcpy(ifr.ifr_hwaddr.sa_data, macAddr->data(), 6);
        } else {
            throw std::runtime_error("Could not set mac addr: invalid mac address format(expected forma: XX:XX:XX:XX:XX)");
        }
        
        if (ioctl(socket, SIOCSIFHWADDR, &ifr) < 0) {
            throw std::runtime_error("Could not set mac address: " + GetLastSysError());
        }
    }

    // Set Net mask address
    if (config.getNetMaskAddress()) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_netmask;
        sin->sin_family = AF_INET;
        inet_pton(AF_INET, config.getMacAddress()->data(), &sin->sin_addr);
        if (ioctl(socket, SIOCSIFNETMASK, &ifr) < 0) {
            throw std::runtime_error("Could not set net mask: " + GetLastSysError());
        }
    }

    // Set Broadcast address
    if (config.getBroadcastAddress()) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_broadaddr;
        sin->sin_family = AF_INET;
        inet_pton(AF_INET, config.getBroadcastAddress()->data(), &sin->sin_addr);
        if (ioctl(socket, SIOCSIFBRDADDR, &ifr) < 0) {
            throw std::runtime_error("Could not set broadcast address: " + GetLastSysError());
        }
    }

    // Set MTU
    if (config.getMTU()) {
        ifr.ifr_ifru.ifru_mtu = *config.getMTU();
        if (ioctl(socket, SIOCGIFMTU, &ifr) < 0) {
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

        if (ioctl(socket, SIOCGIFFLAGS, &ifr) < 0) {
            throw std::runtime_error("Could not set status: " + GetLastSysError());
        }
    }
}

} //! namespace

namespace posnet {

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

void IFaceManager::setConfig(const IFaceConfiguration& config)
{
    assert(m_socket > 0);
    return SetConfig(m_socket, config);
}


void IFaceManager::setConfig(const IFaceConfiguration& config) const
{
    assert(m_socket > 0);
    return SetConfig(m_socket, config);
}

std::vector<IFaceConfiguration> IFaceManager::getConfigs()
{
    assert(m_socket > 0);
    return GetConfigs(m_socket);
}

std::vector<IFaceConfiguration> IFaceManager::getConfigs() const
{
    assert(m_socket > 0);
    return GetConfigs(m_socket);
}

void IFaceManager::enablePromiscuousMode(const std::string_view ifaceName)
{
    if (isIFacePresent(ifaceName)) {
        assert(m_socket > 0);
        EnablePromiscuousMode(ifaceName, m_socket);
    } else {
        throw std::runtime_error("");
    }
}

void IFaceManager::enablePromiscuousMode(const std::string_view ifaceName) const
{
    if (isIFacePresent(ifaceName)) {
        assert(m_socket > 0);
        EnablePromiscuousMode(ifaceName, m_socket);
    } else {
        throw std::runtime_error("");
    }
}

void IFaceManager::disablePromiscuousMode(const std::string_view ifaceName)
{
    if (isIFacePresent(ifaceName)) {
        assert(m_socket > 0);
        DisablePromiscuousMode(ifaceName, m_socket);
    } else {
        throw std::runtime_error("");
    }
}

void IFaceManager::disablePromiscuousMode(const std::string_view ifaceName) const
{
    if (isIFacePresent(ifaceName)) {
        assert(m_socket > 0);
        DisablePromiscuousMode(ifaceName, m_socket);
    } else {
        throw std::runtime_error("");
    }
}

bool IFaceManager::isIFacePresent(const std::string_view ifaceName)
{
    const auto configs = getConfigs();
    const auto it = std::find_if(configs.cbegin(), configs.cend(), [ifaceName](const IFaceConfiguration& config) {
        return (config.getName() && *config.getName() == ifaceName);
    });

    return it != configs.cend();
}

bool IFaceManager::isIFacePresent(const std::string_view ifaceName) const
{
    const auto configs = getConfigs();
    const auto it = std::find_if(configs.cbegin(), configs.cend(), [ifaceName](const IFaceConfiguration& config) {
        return (config.getName() && *config.getName() == ifaceName);
    });
    
    return it != configs.cend();
}


std::ostream& IFaceConfiguration::operator<<(std::ostream& os) const
{
    os << "Net-Iface conf {" << "\n";
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

std::ostream& operator<<(std::ostream& os, const IFaceConfiguration& config)
{
    return config.operator<<(os);
}

std::optional<std::string_view> IFaceConfiguration::getName()
{
    return m_name;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getMacAddress()
{
    return m_macAddr;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getIpAddress()
{
    return m_ipAddr;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getNetMaskAddress()
{
    return m_netMaskAddr;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getBroadcastAddress()
{
    return m_broadcastAddr;
}

std::optional<IFaceConfiguration::IndexType> IFaceConfiguration::getIndex()
{
    return m_index;
}

std::optional<IFaceConfiguration::MTUType> IFaceConfiguration::getMTU()
{
    return m_mtu;
}

std::optional<IFaceConfiguration::Status> IFaceConfiguration::getStatus()
{
    return m_status;
}

std::optional<std::string_view> IFaceConfiguration::getName() const
{
    return m_name;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getMacAddress() const
{
    return m_macAddr;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getIpAddress() const
{
    return m_ipAddr;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getNetMaskAddress() const
{
    return m_netMaskAddr;
}

std::optional<IFaceConfiguration::AddressType> IFaceConfiguration::getBroadcastAddress() const
{
    return m_broadcastAddr;
}

std::optional<IFaceConfiguration::IndexType> IFaceConfiguration::getIndex() const
{
    return m_index;
}

std::optional<IFaceConfiguration::MTUType> IFaceConfiguration::getMTU() const
{
    return m_mtu;
}

std::optional<IFaceConfiguration::Status> IFaceConfiguration::getStatus() const
{
    return m_status;
}

IFaceConfiguration& IFaceConfiguration::setName(const std::string_view name) &
{
    m_name = std::string(name);
    return *this;
}

IFaceConfiguration& IFaceConfiguration::setMacAddress(const AddressType& addr) &
{
    m_macAddr = addr;
    return *this;
}

IFaceConfiguration& IFaceConfiguration::setIpAddress(const AddressType& addr) &
{
    m_ipAddr = addr;
    return *this;
}

IFaceConfiguration& IFaceConfiguration::setNetMaskAddress(const AddressType& addr) &
{
    m_netMaskAddr = addr;
    return *this;
}

IFaceConfiguration& IFaceConfiguration::setBroadcastAddress(const AddressType& addr) &
{
    m_broadcastAddr = addr;
    return *this;
}

IFaceConfiguration& IFaceConfiguration::setIndex(const IndexType index) &
{
    if (!m_index) {
        m_index = index;
    } else {
        throw std::runtime_error("");
    }
    return *this;
}

IFaceConfiguration& IFaceConfiguration::setMTU(const MTUType mtu) &
{
    m_mtu = mtu;
    return *this;
}

IFaceConfiguration& IFaceConfiguration::setStatus(const Status status) &
{
    m_status = status;
    return *this;
}

} //! namespace posnet
