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

class IFaceManager final {
public:
    static constexpr std::string_view LOOP_BACK_INTERFACE_NAME = "lo";
    using AddressType = std::string;
    using IndexType = int;
    using MTUType = int;

    class Configuration final {
    public:
        enum class Status {
            Up, Down,
        };

        Configuration& setName(std::string_view name) && = delete;
        Configuration& setMacAddress(const AddressType& addr) && = delete;
        Configuration& setIpAddress(const AddressType& addr) && = delete;
        Configuration& setNetMaskAddress(const AddressType& addr) && = delete;
        Configuration& setBroadcastAddress(const AddressType& addr) && = delete;
        Configuration& setIndex(IndexType index) && = delete;
        Configuration& setMTU(MTUType mtu) && = delete;
        Configuration& setStatus(Status status) && = delete;

        explicit Configuration() = default;
        Configuration& setName(std::string_view name) &;
        Configuration& setMacAddress(const AddressType& addr) &;
        Configuration& setIpAddress(const AddressType& addr) &;
        Configuration& setNetMaskAddress(const AddressType& addr) &;
        Configuration& setBroadcastAddress(const AddressType& addr) &;
        Configuration& setIndex(IndexType index) &;
        Configuration& setMTU(MTUType mtu) &;
        Configuration& setStatus(Status status) &;

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

    explicit IFaceManager();
    ~IFaceManager();

    std::vector<Configuration> getConfigs();
    std::vector<Configuration> getConfigs() const;
    void setConfig(const Configuration& config);
    void setConfig(const Configuration& config) const;
    
    void enablePromiscuousMode(std::string_view ifaceName);
    void enablePromiscuousMode(std::string_view ifaceName) const;

    void disablePromiscuousMode(std::string_view ifaceName);
    void disablePromiscuousMode(std::string_view ifaceName) const;

private:
    bool isIFacePresent(std::string_view ifaceName);
    bool isIFacePresent(std::string_view ifaceName) const;
    int m_socket;
};

std::ostream& operator<<(std::ostream& os, const IFaceManager::Configuration& config);

} //! namespace posnet

#endif //! VS_IFACE_MANAGER_H