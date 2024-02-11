#ifndef VS_SCOPED_LOCK_H
#define VS_SCOPED_LOCK_H

#include <optional>

namespace utils {

/**
* @brief This class represents RAII util.
* @details This class expands ability of boots::scoped_prt.
* This class just do two things: lock in constructor(by functor) and unlock(by functor) resources.
* see for more information: https://www.boostcpp.org/doc/libs/1_48_0/libs/smart_ptr/scoped_ptr.htm
* @tparam C - lock functor(callback) type.
* @tparam T - unlock functor(callback) type.
* @warning Size of this class = sizeof(T2).
* @warning Make sure that we don`t lock a really big object in your functor.
* @warning Make sure that we don`t lock a shared resources.
*/
template<typename T>
class ScopedLock final {
public:
    static_assert(std::is_invocable_v<T>, "Attempt to construct ScopeLock with invocable type T");
    using UnlockFunctorType = T;

    template<typename C, typename D = std::enable_if_t<std::is_invocable_v<C>, int>>
    explicit ScopedLock(C lockCallback, T unlockCallback);
    explicit ScopedLock(T unlockCallback);
    ~ScopedLock();
    void unlock();
    operator bool() const;
    bool wasUnlocked() const;

    ScopedLock(const ScopedLock&) = delete;
    ScopedLock(ScopedLock&&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;
    ScopedLock& operator=(ScopedLock&&) = delete;

private:
    std::optional<UnlockFunctorType> m_unlockCallback;
};

template<typename T>
inline ScopedLock<T>::ScopedLock(T unlockCallback) :
    m_unlockCallback(std::move(unlockCallback))
{
    static_assert(std::is_invocable_v<T>);
}

template<typename T>
template<typename C, typename D>
inline ScopedLock<T>::ScopedLock(C lockCallback, T unlockCallback) :
    m_unlockCallback(std::move(unlockCallback))
{
    static_assert(std::is_invocable_v<C> && std::is_invocable_v<T>);
    lockCallback();
}

template<typename T>
inline ScopedLock<T>::~ScopedLock()
{
    unlock();
}

template<typename T>
inline void ScopedLock<T>::unlock()
{
    if (!wasUnlocked()) {
        m_unlockCallback.value()();
        m_unlockCallback.reset();
    }
}

template<typename T>
inline ScopedLock<T>::operator bool() const
{
    return wasUnlocked();
}

template<typename T>
inline bool ScopedLock<T>::wasUnlocked() const
{
    return !m_unlockCallback.has_value();
}

} //! namespace utils

#endif //! VS_SCOPED_LOCK_H
