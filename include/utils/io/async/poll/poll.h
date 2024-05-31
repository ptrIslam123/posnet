#ifndef VS_IO_POLL_H
#define VS_IO_POLL_H

#include "include/utils/stage_controller.h"
#include "include/utils/synchronizers/synchronized.h"

#include <poll.h>
#include <unistd.h>

#include <exception>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <utility>
#include <chrono>
#include <optional>
#include <atomic>
#include <limits>

#include <cstdint>

namespace posnet::utils::io::async::posix {

class BadPoll final : public std::exception {
public:
    explicit BadPoll(std::string_view msg);
    virtual const char* what() const noexcept;

private:
    std::string m_msg;
};

class Poll final {
public:
    static constexpr auto DEFAULT_WAIT_TIMEOUT = std::chrono::milliseconds(100);
    static constexpr auto DEFAULT_DELETOR = [](int sockfd) { close(sockfd); };
    static constexpr auto MAX_MONITOR_DESCRIPTORS = std::numeric_limits<std::uint64_t>::max();

    enum class IOEventType : std::int8_t {
        Read = POLLIN, 
        Write = POLLOUT,
        ReadWrite = POLLIN | POLLOUT,
    };

    enum class State {
        Started, InProcessStopping, Stopped,
    };

    using IOEventHandlerType = std::function<void(int /*sockfd*/)>;
    using DeletorType = std::function<void(int /*sockdf*/)>;

    explicit Poll(int size=0);
    ~Poll();
    Poll(const Poll& ) = delete;
    Poll(Poll&& ) noexcept = delete;
    Poll& operator=(const Poll& ) = delete;
    Poll& operator=(Poll&& ) noexcept = delete;

    bool onReadEvent(
            int sockfd,
            IOEventHandlerType&& callback,
            DeletorType&& deletor = DEFAULT_DELETOR
    );

    bool onWriteEvent(
            int sockfd,
            IOEventHandlerType&& callback,
            DeletorType&& deletor = DEFAULT_DELETOR
    );

    bool removeCallback(int sockfd, IOEventType ioEvent = IOEventType::ReadWrite);
    bool start(const std::chrono::milliseconds& waitTimeout = DEFAULT_WAIT_TIMEOUT);
    bool stop();

private:
    bool addCallback(
            int sockfd,
            IOEventType ioEvent,
            IOEventHandlerType&& callbackOnRead,
            IOEventHandlerType&& callbackOnWrite,
            DeletorType&& deletor = DEFAULT_DELETOR
    );

    struct IOListenerData final {
        std::optional<IOEventHandlerType> callbackOnRead;
        std::optional<IOEventHandlerType> callbackOnWrite;
        DeletorType deletor;
    };

    struct AppendEventDataHolder final {
        std::optional<IOEventHandlerType> callbackOnRead;
        std::optional<IOEventHandlerType> callbackOnWrite;
        DeletorType deletor;
        std::uint64_t timestamp;
    };

    struct RemoveEventDataHolder final {
        IOEventType ioEvent;
        std::uint64_t timestamp;
    };

    using PollSetType = std::vector<struct pollfd>;
    using CallbackTableType = std::unordered_map<int, IOListenerData>;
    using AppendListenerTableType = std::unordered_map<int/*sockfd*/, AppendEventDataHolder>;
    using RemoveListenerTableType = std::unordered_map<int/*sockfd*/, RemoveEventDataHolder>;

    void updateIOPoll();
    bool updateIOPollCallbackTable(AppendListenerTableType& appendTable, RemoveListenerTableType& removeTable);

    std::uint64_t getCurrentTimestamp();
    void resetTimestamp();
    bool mayChangeIOPoll() const;

    CallbackTableType m_mainCallbackTable;
    PollSetType m_mainIOPoll;
    std::atomic<std::uint64_t> m_ioPollSizel;

    Synchronized<AppendListenerTableType> m_appendTable;
    std::atomic<std::uint64_t> m_appendTableSize;

    Synchronized<RemoveListenerTableType> m_removeTable;
    std::atomic<std::uint64_t> m_removeTableSize;

    std::atomic<std::uint64_t> m_timestamp;
    utils::StageController<State> m_state;
};

} //! namespace posnet::utils::io::async::posix

#endif //! VS_IO_POLL_H
