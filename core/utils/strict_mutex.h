#ifndef VS_STRICT_MUTEX_H
#define VS_STRICT_MUTEX_H

#include <mutex>
#include <shared_mutex>
#include <cassert>

namespace utils {

/**
* @brief Implement strict mutex entity.
* @details MOTIVATION: usual mutex is a synchronization primitive, and everybody has to lock it(before using shared data) and
* unlock it(after finishing work with shared data). But it is just an agreement, nothing more.
* Somebody can forget call lock and/or unlock before/after using the shared data. But this class can help solve this problem.
* This class combines the shared data and the mutex into one entity, and nobody can access to the shared data without locking it and
* nobody can forget to call unlock(after finishing work with the shared data).
* @example {
*               StrictMutex<SharedDataStruct> sharedData;
*               auto sharedDataholder = sharedData.lock(); // we got a unique access to the shared data
*               sharedDataholder->counter++;                // increment counter of shred data thread safety
*               // we can call `sharedDataholder->unlock(); or this  method will be called automatically when we out of scope
*           }
* @warning Do not forget that the @MutexGuard instance is just RAII wrapped(it contains reference to the shred data and the mutex of @StrictMutex).
* It means that you have to provide enough lifetime of the @StrictMutex for @MutexGuard.
* @tparam T - Type of shared data between multi threads.
* @tparam M - Type of mutex.
*/
template<typename T, typename M = std::mutex>
class StrictMutex final {
public:
    class [[nodiscard]] MutexGuard {
    public:
        MutexGuard(const MutexGuard&) = delete;
        MutexGuard& operator=(const MutexGuard&) = delete;
        MutexGuard(MutexGuard&&) noexcept;
        MutexGuard& operator=(MutexGuard&&) noexcept;
        ~MutexGuard();

        T* operator->() && = delete;
        T& operator*() && = delete;

        T* operator->() &;
        const T* operator->() const &;
        T& operator*() &;
        const T& operator*() const &;

        void unlock();
        operator bool() const;
        bool isLocked() const;

    private:
        friend StrictMutex;
        explicit MutexGuard(T* data, M* mutex);

        T* m_data;
        M* m_mutex;
    };

    template<typename ... Args>
    StrictMutex(Args&& ... args);

    MutexGuard lock() &;
    const MutexGuard lock() const &;

private:
    T m_data;
    M m_mutex;
};


template<typename T>
class StrictMutex<T, std::shared_mutex> final {
public:
    class [[nodiscard]] SharedMutexGuard {
    public:
        SharedMutexGuard(const SharedMutexGuard&) = delete;
        SharedMutexGuard& operator=(const SharedMutexGuard&) = delete;
        SharedMutexGuard(SharedMutexGuard&&) noexcept;
        SharedMutexGuard& operator=(SharedMutexGuard&&) noexcept;
        ~SharedMutexGuard();

        const T* operator->() && = delete;
        const T& operator*() && = delete;

        const T* operator->() const &;
        const T& operator*() const &;

        void unlock();
        operator bool() const;
        bool isLocked() const;

    private:
        friend StrictMutex;
        explicit SharedMutexGuard(const T* data, std::shared_mutex* mutex);

        const T* m_data;
        std::shared_mutex* m_mutex;
    };

    class UniqueMutexGuard {
    public:
        UniqueMutexGuard(const UniqueMutexGuard&) = delete;
        UniqueMutexGuard& operator=(const UniqueMutexGuard&) = delete;
        UniqueMutexGuard(UniqueMutexGuard&&) noexcept;
        UniqueMutexGuard& operator=(UniqueMutexGuard&&) noexcept;
        ~UniqueMutexGuard();

        T* operator->() &;
        T& operator*() &;

        void unlock();
        operator bool() const;
        bool isLocked() const;

    private:
        friend StrictMutex;
        explicit UniqueMutexGuard(T* data, std::shared_mutex* mutex);

        T* m_data;
        std::shared_mutex* m_mutex;
    };

    template<typename ... Args>
    StrictMutex(Args&& ... args);

    UniqueMutexGuard lockOnWrite();
    SharedMutexGuard lockOnRead();

private:
    T m_data;
    std::shared_mutex m_mutex;
};

template<typename T>
template<typename ...Args>
inline StrictMutex<T, std::shared_mutex>::StrictMutex(Args&& ...args):
m_data(std::forward<Args>(args) ... ),
m_mutex()
{}

template<typename T, typename M>
template<typename ...Args>
inline StrictMutex<T, M>::StrictMutex(Args&& ...args):
m_data(std::forward<Args>(args) ... ),
m_mutex()
{}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::UniqueMutexGuard(UniqueMutexGuard&& other) noexcept
{
    this->operator=(std::move(other));
}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::SharedMutexGuard::SharedMutexGuard(SharedMutexGuard&& other) noexcept
{
    this->operator=(std::move(other));
}

template<typename T, typename M>
inline StrictMutex<T, M>::MutexGuard::MutexGuard(MutexGuard&& other) noexcept
{
    this->operator=(std::move(other));
}

template<typename T>
inline typename StrictMutex<T, std::shared_mutex>::UniqueMutexGuard&
StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::operator=(UniqueMutexGuard&& other) noexcept
{
    m_data = other.m_data;
    m_mutex = other.m_mutex;
    other.m_data = nullptr;
    other.m_mutex = nullptr;
    return *this;
}

template<typename T>
inline typename StrictMutex<T, std::shared_mutex>::SharedMutexGuard&
StrictMutex<T, std::shared_mutex>::SharedMutexGuard::operator=(SharedMutexGuard&& other) noexcept
{
    m_data = other.m_data;
    m_mutex = other.m_mutex;
    other.m_data = nullptr;
    other.m_mutex = nullptr;
    return *this;
}

template<typename T, typename M>
inline typename StrictMutex<T, M>::MutexGuard&
StrictMutex<T, M>::MutexGuard::operator=(MutexGuard&& other) noexcept
{
    m_data = other.m_data;
    m_mutex = other.m_mutex;
    other.m_data = nullptr;
    other.m_mutex = nullptr;
    return *this;
}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::~UniqueMutexGuard()
{
    if (isLocked()) {
        unlock();
    }
}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::SharedMutexGuard::~SharedMutexGuard()
{
    if (isLocked()) {
        unlock();
    }
}

template<typename T, typename M>
inline StrictMutex<T, M>::MutexGuard::~MutexGuard()
{
    if (isLocked()) {
        unlock();
    }
}

template<typename T>
inline T* StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::operator->() &
{
    assert(isLocked());
    return m_data;
}

template<typename T, typename M>
inline T* StrictMutex<T, M>::MutexGuard::operator->() &
{
    assert(isLocked());
    return m_data;
}

template<typename T>
inline const T* StrictMutex<T, std::shared_mutex>::SharedMutexGuard::operator->() const &
{
    assert(isLocked());
    return m_data;
}

template<typename T, typename M>
inline const T* StrictMutex<T, M>::MutexGuard::operator->() const &
{
    assert(isLocked());
    return m_data;
}

template<typename T, typename M>
inline T& StrictMutex<T, M>::MutexGuard::operator*() &
{
    assert(isLocked());
    return *m_data;
}

template<typename T>
inline T& StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::operator*() &
{
    assert(isLocked());
    return *m_data;
}

template<typename T>
inline const T& StrictMutex<T, std::shared_mutex>::SharedMutexGuard::operator*() const &
{
    assert(isLocked());
    return *m_data;
}

template<typename T, typename M>
inline const T& StrictMutex<T, M>::MutexGuard::operator*() const &
{
    assert(isLocked());
    return *m_data;
}

template<typename T>
inline void StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::unlock()
{
    assert(isLocked());
    m_mutex->unlock();
    m_mutex = nullptr;
    m_data = nullptr;
}

template<typename T>
inline void StrictMutex<T, std::shared_mutex>::SharedMutexGuard::unlock()
{
    assert(isLocked());
    m_mutex->unlock_shared();
    m_mutex = nullptr;
    m_data = nullptr;
}

template<typename T, typename M>
inline void StrictMutex<T, M>::MutexGuard::unlock()
{
    assert(isLocked());
    m_mutex->unlock();
    m_mutex = nullptr;
    m_data = nullptr;
}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::operator bool() const
{
    return isLocked();
}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::SharedMutexGuard::operator bool() const
{
    return isLocked();
}

template<typename T, typename M>
inline StrictMutex<T, M>::MutexGuard::operator bool() const
{
    return isLocked();
}

template<typename T>
inline bool StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::isLocked() const
{
    return m_data && m_mutex;
}

template<typename T>
inline bool StrictMutex<T, std::shared_mutex>::SharedMutexGuard::isLocked() const
{
    return m_data && m_mutex;
}

template<typename T, typename M>
inline bool StrictMutex<T, M>::MutexGuard::isLocked() const
{
    return m_data && m_mutex;
}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::UniqueMutexGuard::UniqueMutexGuard(
    T* const data,
    std::shared_mutex* const mutex
) :
m_data(data),
m_mutex(mutex)
{
    assert(m_data && m_mutex);
    m_mutex->lock();
}

template<typename T>
inline StrictMutex<T, std::shared_mutex>::SharedMutexGuard::SharedMutexGuard(
        const T* const data,
        std::shared_mutex* const mutex
):
m_data(data),
m_mutex(mutex)
{
    assert(m_data && m_mutex);
    m_mutex->lock_shared();
}

template<typename T, typename M>
inline StrictMutex<T, M>::MutexGuard::MutexGuard(T* const data, M* const mutex):
m_data(data),
m_mutex(mutex)
{
    assert(m_data && m_mutex);
    m_mutex->lock();
}

template<typename T, typename M>
inline typename StrictMutex<T, M>::MutexGuard StrictMutex<T, M>::lock() &
{
    return MutexGuard{ &m_data, &m_mutex };
}

template<typename T, typename M>
inline const typename StrictMutex<T, M>::MutexGuard StrictMutex<T, M>::lock() const &
{
    return MutexGuard{ &m_data, &m_mutex };
}

template<typename T>
inline typename StrictMutex<T, std::shared_mutex>::UniqueMutexGuard
StrictMutex<T, std::shared_mutex>::lockOnWrite()
{
    return UniqueMutexGuard{ &m_data, &m_mutex };
}

template<typename T>
inline typename StrictMutex<T, std::shared_mutex>::SharedMutexGuard
StrictMutex<T, std::shared_mutex>::lockOnRead()
{
    return SharedMutexGuard{ &m_data, &m_mutex };
}

} //! namespace utils 

#endif // !VS_STRICT_MUTEX_H
