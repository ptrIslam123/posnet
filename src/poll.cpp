#include "include/utils/io/async/poll/poll.h"
#include "include/utils/assert.h"

#include <cerrno>

#include <algorithm>
#include <limits>
#include <cstring>
#include <cassert>

namespace posnet::utils::io::async::posix {

BadPoll::BadPoll(const std::string_view msg):
m_msg(msg)
{}

const char* BadPoll::what() const noexcept
{
    return m_msg.data();
}

Poll::Poll(const int size):
m_mainCallbackTable(),
m_mainIOPoll(),
m_ioPollSizel(0),
m_appendTable(),
m_appendTableSize(0),
m_removeTable(),
m_removeTableSize(0),
m_timestamp(0),
m_state(State::Stopped)
{}

Poll::~Poll()
{
    stop();
}

bool Poll::onReadEvent(
        const int sockfd,
        IOEventHandlerType&& callback,
        DeletorType&& deletor
)
{
    return addCallback(sockfd, IOEventType::Read, std::move(callback), IOEventHandlerType{}, std::move(deletor));
}

bool Poll::onWriteEvent(
        int sockfd,
        IOEventHandlerType&& callback,
        DeletorType&& deletor
)
{
    return addCallback(sockfd, IOEventType::Write, IOEventHandlerType{}, std::move(callback), std::move(deletor));
}


bool Poll::addCallback(
        const int sockfd,
        const IOEventType ioEvent,
        IOEventHandlerType&& callbackOnRead,
        IOEventHandlerType&& callbackOnWrite,
        DeletorType&& deletor
)
{
    auto added = false;
    AppendEventDataHolder dataHolder;
    dataHolder.timestamp = getCurrentTimestamp();

    const auto event = static_cast<decltype(POLLIN)>(ioEvent);
    if (event & POLLIN) {
        dataHolder.callbackOnRead = std::move(callbackOnRead);
    }

    if (event & POLLOUT) {
        dataHolder.callbackOnWrite = std::move(callbackOnWrite);
    }

    m_appendTable.withLock([this, &added, sockfd, dataHolder = std::move(dataHolder)](AppendListenerTableType& table) {
        if (mayChangeIOPoll()) {
            if (auto size = m_appendTableSize.fetch_add(1);
                    size < MAX_MONITOR_DESCRIPTORS) {
                table.emplace(sockfd, std::move(dataHolder));
                added = true;
            } else {
                m_appendTableSize.store(size);
            }
        }
    });

    return added;
}

bool Poll::removeCallback(const int sockfd, const IOEventType ioEvent)
{
    auto removed = false;
    RemoveEventDataHolder dataHolder;
    dataHolder.timestamp = getCurrentTimestamp();
    dataHolder.ioEvent = ioEvent;

    m_removeTable.withLock([this, &removed, sockfd, dataHolder = std::move(dataHolder)](RemoveListenerTableType& table) {
        if (mayChangeIOPoll()) {
            if (auto size = m_removeTableSize.fetch_add(1);
                    size < MAX_MONITOR_DESCRIPTORS) {
                table.emplace(sockfd, std::move(dataHolder));
                removed = true;
            } else {
                m_removeTableSize.store(size);
            }
        }
    });

    return removed;
}

bool Poll::start(const std::chrono::milliseconds& waitTimeout)
{
    if (!m_state.gotoNextStageIfCurrentStage(State::Stopped, State::Started)) {
        return false;
    }

    while(true) {
        updateIOPoll();

        if (m_state.getCurrentStage() != State::Started) { // check state before waiting
            break;
        }


        m_ioPollSizel.store(m_mainIOPoll.size());
        const auto result = poll(m_mainIOPoll.data(), m_mainIOPoll.size(), static_cast<int>(waitTimeout.count()));
        ASSERTION(result >= 0, BadPoll, "Poll syscall failed: " + std::string(strerror(errno)))
        if (result == 0) {
            // wake up with timeout
            continue;
        }

        if (m_state.getCurrentStage() != State::Started) { // check state after waiting
            break;
        }

        CallbackTableType::const_iterator it;
        for (auto& poll : m_mainIOPoll) {
            const auto sockfd = poll.fd;
            if (poll.revents & POLLIN) {
                it = m_mainCallbackTable.find(sockfd);
                if (it != m_mainCallbackTable.cend() && it->second.callbackOnRead.has_value()) {
                    it->second.callbackOnRead->operator()(sockfd);
                }
            }

            if (poll.revents & POLLOUT) {
                it = m_mainCallbackTable.find(sockfd);
                if (it != m_mainCallbackTable.cend() && it->second.callbackOnWrite.has_value()) {
                    it->second.callbackOnWrite->operator()(sockfd);
                }
            }
        }
    }

    assert(m_state.getCurrentStage() == State::InProcessStopping);

    for (auto && [sockfd, ioDataHolder] : m_mainCallbackTable) {
        ioDataHolder.deletor(sockfd);
    }
    m_mainCallbackTable.clear();
    m_mainIOPoll.clear();

    m_state.setNextStageStrongly(State::Stopped);
    return true;
}

bool Poll::stop()
{
    return m_state.gotoNextStageIfCurrentStage(State::Started, State::InProcessStopping);
}

void Poll::updateIOPoll()
{
    auto updated = false;
    m_removeTable.withLock([this, &updated](RemoveListenerTableType& removeTable) {
        m_appendTable.withLock([this, &updated, &removeTable](AppendListenerTableType& appendTable) {
            updated = updateIOPollCallbackTable(appendTable, removeTable);
        });

        resetTimestamp();
    });

    if (updated) {
        m_mainIOPoll.clear();

        for (const auto& [sockfd, ioListenerData] : m_mainCallbackTable) {
            struct pollfd poll;
            std::memset(&poll, 0, sizeof(poll));

            poll.fd = sockfd;
            if (ioListenerData.callbackOnRead.has_value()) {
                poll.events |= POLLIN;
            }

            if (ioListenerData.callbackOnWrite.has_value()) {
                poll.events |= POLLOUT;
            }

            m_mainIOPoll.push_back(poll);
        }
    }
}

bool Poll::updateIOPollCallbackTable(AppendListenerTableType& appendTable, RemoveListenerTableType& removeTable)
{
    auto doNotNeedToUpdateMainIOPoll = appendTable.empty() && removeTable.empty();

    for (auto appendIt = appendTable.begin(); appendIt != appendTable.end();) {
        auto& [sockfd, appendData] = *appendIt;
        if (removeTable.find(sockfd) == removeTable.cend()) {
            auto it = m_mainCallbackTable.find(sockfd);
            if (it == m_mainCallbackTable.cend()) {
                IOListenerData listenerData = {
                    .callbackOnRead = std::move(appendData.callbackOnRead),
                    .callbackOnWrite = std::move(appendData.callbackOnWrite),
                    .deletor = std::move(appendData.deletor),
                };
                m_mainCallbackTable.emplace(sockfd, std::move(listenerData));
            } else {
                IOListenerData& listenerData = it->second;
                if (appendData.callbackOnRead.has_value()) {
                    listenerData.callbackOnRead = std::move(appendData.callbackOnRead);
                }

                if (appendData.callbackOnWrite.has_value()) {
                    listenerData.callbackOnWrite = std::move(appendData.callbackOnWrite);
                }

                listenerData.deletor = std::move(appendData.deletor);
            }

            appendIt = appendTable.erase(appendIt);
        } else {
            ++appendIt;
        }
    }

    for (auto removeIt = removeTable.begin(); removeIt != removeTable.end(); ) {
        auto& [sockfd, removeData] = *removeIt;
        if (appendTable.find(sockfd) == appendTable.cend()) {
            const auto event = static_cast<decltype(POLLIN)>(removeData.ioEvent);
            auto it = m_mainCallbackTable.find(sockfd);
            if (it == m_mainCallbackTable.cend()) {
                continue;
            }

            auto& [_, listenerData] = *it;
            if (event & POLLIN) {
                listenerData.callbackOnRead.reset();
            }

            if (event & POLLOUT) {
                listenerData.callbackOnWrite.reset();
            }

            if (!listenerData.callbackOnRead.has_value() && !listenerData.callbackOnWrite.has_value()) {
                m_mainCallbackTable.erase(it);
            }

            removeIt = removeTable.erase(removeIt);
        } else {
            ++removeIt;
        }
    }

    for (auto& [sockfd, removeData] : removeTable) {
        const auto appendIt = appendTable.find(sockfd);
        if (appendIt != appendTable.end()) {}
        else {
            assert(false);
            continue;
        }
        auto& appendData = appendIt->second;
        auto it = m_mainCallbackTable.find(sockfd);

        if (removeData.timestamp > appendData.timestamp && it != m_mainCallbackTable.cend()) {
            // aply removing
            const auto event = static_cast<decltype(POLLIN)>(removeData.ioEvent);
            auto& [_, listenerData] = *it;
            if (event & POLLIN) {
                listenerData.callbackOnRead.reset();
            }

            if (event & POLLOUT) {
                listenerData.callbackOnWrite.reset();
            }

            if (!listenerData.callbackOnRead.has_value() && !listenerData.callbackOnWrite.has_value()) {
                m_mainCallbackTable.erase(it);
            }
        } else if (removeData.timestamp < appendData.timestamp && it != m_mainCallbackTable.cend()) {
            // aply appending
            if (it != m_mainCallbackTable.cend()) {
                IOListenerData listenerData = {
                    .callbackOnRead = std::move(appendData.callbackOnRead),
                    .callbackOnWrite = std::move(appendData.callbackOnWrite),
                    .deletor = std::move(appendData.deletor),
                };
                m_mainCallbackTable.emplace(sockfd, std::move(listenerData));
            } else {
                IOListenerData& listenerData = it->second;
                if (appendData.callbackOnRead.has_value()) {
                    listenerData.callbackOnRead = std::move(appendData.callbackOnRead);
                }

                if (appendData.callbackOnWrite.has_value()) {
                    listenerData.callbackOnWrite = std::move(appendData.callbackOnWrite);
                }

                listenerData.deletor = std::move(appendData.deletor);
            }
        } else {
            // Do nothing
        }
    }

    m_appendTableSize.store(0);
    m_removeTableSize.store(0);
    appendTable.clear();
    removeTable.clear();
    return !doNotNeedToUpdateMainIOPoll;
}

std::uint64_t Poll::getCurrentTimestamp()
{
    constexpr auto timestampLimit = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t currTime = 0;
    do {
        currTime = m_timestamp.load();
        if (currTime < timestampLimit) {
            if (m_timestamp.compare_exchange_weak(currTime, currTime + 1)) {
                break;
            }
        } else {
            assert(false);
        }
    } while(true);

    return ++currTime;
}

bool Poll::mayChangeIOPoll() const
{
    return (m_ioPollSizel.load() + m_appendTableSize.load() + m_removeTableSize.load()  + 1) < MAX_MONITOR_DESCRIPTORS;
}

void Poll::resetTimestamp()
{
    m_timestamp.store(0);
}

} //! namespace posnet::utils::io::async::posix
