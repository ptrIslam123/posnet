#ifndef VS_IFACE_MANAGER_H
#define VS_IFACE_MANAGER_H

#include <ostream>
#include <optional>
#include <vector>
#include <string_view>
#include <string>

/**
* @brief
*/
namespace posnet {

class IFaceConfiguration final {
public:
    static constexpr std::string_view LOOP_BACK_INTERFACE_NAME = "lo";
    using AddressType = std::string;
    using IndexType = int;
    using MTUType = int;

    enum class Status {
        Up, Down,
    };

    IFaceConfiguration& setName(std::string_view name) && = delete;
    IFaceConfiguration& setMacAddress(const AddressType& addr) && = delete;
    IFaceConfiguration& setIpAddress(const AddressType& addr) && = delete;
    IFaceConfiguration& setNetMaskAddress(const AddressType& addr) && = delete;
    IFaceConfiguration& setBroadcastAddress(const AddressType& addr) && = delete;
    IFaceConfiguration& setIndex(IndexType index) && = delete;
    IFaceConfiguration& setMTU(MTUType mtu) && = delete;
    IFaceConfiguration& setStatus(Status status) && = delete;

    explicit IFaceConfiguration() = default;
    IFaceConfiguration& setName(std::string_view name) &;
    IFaceConfiguration& setMacAddress(const AddressType& addr) &;
    IFaceConfiguration& setIpAddress(const AddressType& addr) &;
    IFaceConfiguration& setNetMaskAddress(const AddressType& addr) &;
    IFaceConfiguration& setBroadcastAddress(const AddressType& addr) &;
    IFaceConfiguration& setIndex(IndexType index) &;
    IFaceConfiguration& setMTU(MTUType mtu) &;
    IFaceConfiguration& setStatus(Status status) &;

    std::optional<std::string_view> getName();
    std::optional<AddressType> getMacAddress();
    std::optional<AddressType> getIpAddress();
    std::optional<AddressType> getNetMaskAddress();
    std::optional<AddressType> getBroadcastAddress();
    std::optional<IndexType> getIndex();
    std::optional<MTUType> getMTU();
    std::optional<Status> getStatus();

    std::optional<std::string_view> getName() const;
    std::optional<AddressType> getMacAddress() const;
    std::optional<AddressType> getIpAddress() const;
    std::optional<AddressType> getNetMaskAddress() const;
    std::optional<AddressType> getBroadcastAddress() const;
    std::optional<IndexType> getIndex() const;
    std::optional<MTUType> getMTU() const;
    std::optional<Status> getStatus() const;

    std::ostream& operator<<(std::ostream& os) const;

private:
    std::optional<std::string> m_name;
    std::optional<AddressType> m_macAddr;
    std::optional<AddressType> m_ipAddr;
    std::optional<AddressType> m_netMaskAddr;
    std::optional<AddressType> m_broadcastAddr;
    std::optional<IndexType> m_index;
    std::optional<MTUType> m_mtu;
    std::optional<Status> m_status;
};

std::ostream& operator<<(std::ostream& os, const IFaceConfiguration& config);

class IFaceManager final {
public:
    explicit IFaceManager();
    ~IFaceManager();

    std::vector<IFaceConfiguration> getConfigs();
    std::vector<IFaceConfiguration> getConfigs() const;
    void setConfig(const IFaceConfiguration& config);
    void setConfig(const IFaceConfiguration& config) const;
    
    void enablePromiscuousMode(std::string_view ifaceName);
    void enablePromiscuousMode(std::string_view ifaceName) const;

    void disablePromiscuousMode(std::string_view ifaceName);
    void disablePromiscuousMode(std::string_view ifaceName) const;

private:
    bool isIFacePresent(std::string_view ifaceName);
    bool isIFacePresent(std::string_view ifaceName) const;
    int m_socket;
};

} //! namespace posnet

#endif //! VS_IFACE_MANAGER_H