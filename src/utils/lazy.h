#ifndef VS_LAZY_H
#define VS_LAZY_H

#include <array>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <exception>
#include <typeinfo>
#include <type_traits>
#include <new>
#include <memory>

namespace posnet::utils {

/**
 * @brief This class is used for lazy initialization. This class implement lazy initialization idiom.
 * @details This class contains buffer, enough size, to store object of type T.
 * The size of this class is always sizeof(T) + 1 byte(for binary flag)
 * @warning  This class really call constructor of the object type T then user call Lazy& operator=(T) or Lazy(T).
 * We should not access to internal object of this class before call Lazy& operator=(T) or Lazy(T).
 * Accessing to internal object of this class is Undefined behavior!
 * @warning !This class IS NOT THREAD SAFE.
 */
template<typename T>
class LazyConstructedObject final {
public:
    using ValueType = T;

    explicit LazyConstructedObject();
    explicit LazyConstructedObject(const T& value);
    explicit LazyConstructedObject(T&& value);
    ~LazyConstructedObject();
    LazyConstructedObject(const LazyConstructedObject&);
    LazyConstructedObject(LazyConstructedObject&&) noexcept;
    LazyConstructedObject& operator=(LazyConstructedObject&&) & noexcept;
    LazyConstructedObject& operator=(const T& value) &;
    LazyConstructedObject& operator=(T&& value) &;
    LazyConstructedObject& operator=(const LazyConstructedObject&) &;
    template<typename ... Args>
    void operator()(Args&& ... args);

    template<typename ... Args>
    void construct(Args&& ... args);
    /**
    * @brief Drop this object
    * @warning Access to internal object of this class after call drop function is undefined behavior!
    */
    void destruct();
    template<typename F>
    void destructNoThrow(F throwHandler);

    //! These function(methods) use in order to access to internal object of this class
    T& operator*() & noexcept(false);
    const T& operator*() const & noexcept(false);

    T* operator->() & noexcept(false);
    const T* operator->() const & noexcept(false);

    T* get() & noexcept(false);
    const T* get() const & noexcept(false);

    T into_value() & noexcept(false);
    T into_value() const& noexcept(false);
    T into_value() && noexcept(false);

    operator bool() const;
    bool isEmpty() const;

private:
    const std::string& getFullClassName() const;

    std::array<uint8_t, sizeof(T)> m_buffer; //! storage that contains object T
    bool m_isEmpty; //! flag to determine that this class was initialized
};

template<typename T>
LazyConstructedObject<T>::LazyConstructedObject():
m_buffer(),
m_isEmpty(true)
{
    std::memset(m_buffer.data(), 0, m_buffer.size());
}

template<typename T>
LazyConstructedObject<T>::LazyConstructedObject(const T& value) :
m_buffer(),
m_isEmpty(false)
{
    std::memset(m_buffer.data(), 0, m_buffer.size());
    new(m_buffer.data()) T(value);
}

template<typename T>
LazyConstructedObject<T>::LazyConstructedObject(T&& value) :
m_buffer(),
m_isEmpty(false)
{
    std::memset(m_buffer.data(), 0, m_buffer.size());
    new(m_buffer.data()) T(std::move(value));
}

template<typename T>
LazyConstructedObject<T>::LazyConstructedObject(const LazyConstructedObject& other):
m_buffer(),
m_isEmpty(true)
{
    if (!other) {
        return;
    }

    std::memset(m_buffer.data(), 0, m_buffer.size());
    new(m_buffer.data()) T(other.into_value());
}

template<typename T>
LazyConstructedObject<T>::LazyConstructedObject(LazyConstructedObject&& other) noexcept:
m_buffer(),
m_isEmpty(true)
{
    if (!other) {
        return;
    }

    std::memset(m_buffer.data(), 0, m_buffer.size());
    new(m_buffer.data()) T(std::move(other.into_value()));
    other.m_isEmpty = true;
}

template<typename T>
LazyConstructedObject<T>::~LazyConstructedObject()
{
    if (!isEmpty()) {
        destruct();
    }
}

template<typename T>
LazyConstructedObject<T>& LazyConstructedObject<T>::operator=(const T& value) &
{
    m_isEmpty = false;
    new(get()) T(value);
    return *this;
}

template<typename T>
LazyConstructedObject<T>& LazyConstructedObject<T>::operator=(T&& value) &
{
    m_isEmpty = false;
    new(get()) T(std::move(value));
    return *this;
}

template<typename T>
LazyConstructedObject<T>& LazyConstructedObject<T>::operator=(const LazyConstructedObject<T>& other) &
{
    if (!other) {
        return *this;
    }

    if (!isEmpty()) {
        destruct();
    }

    m_isEmpty = false;
    std::memset(m_buffer.data(), 0, m_buffer.size());
    new(get()) T(other.into_value());
    return *this;
}

template<typename T>
LazyConstructedObject<T>& LazyConstructedObject<T>::operator=(LazyConstructedObject<T>&& other) & noexcept
{
    if (!other) {
        return *this;
    }

    if (!isEmpty()) {
        destruct();
    }

    m_isEmpty = false;
    std::memset(m_buffer.data(), 0, m_buffer.size());
    new(get()) T(std::move(other.into_value()));
    return *this;
}

template<typename T>
T& LazyConstructedObject<T>::operator*() &
{
    return *get();
}

template<typename T>
const T& LazyConstructedObject<T>::operator*() const &
{
    return *get();
}

template<typename T>
T* LazyConstructedObject<T>::operator->() &
{
    return get();
}

template<typename T>
const T* LazyConstructedObject<T>::operator->() const &
{
    return get();
}

template<typename T>
T* LazyConstructedObject<T>::get() &
{
    if (!isEmpty()) {
        return reinterpret_cast<T*>(m_buffer.data());
    } else {
        throw std::runtime_error(getFullClassName() + "::" + __FUNCTION__ +
            ": Attempt to access to empty object");
    }

}

template<typename T>
const T* LazyConstructedObject<T>::get() const &
{
    if (!isEmpty()) {
        //! A lot of developers believe that `const_cast` should only be used when it is necessary and safe to do so.
        //! I think this case is an appropriate and safe for using `const_cast`.
        //! We suppose that the user is sure that it is not empty class.
        //! If this statement is true, it means that m_buffer already contains the constructed object T and
        //! we may interpret this buffer as object of type T.
        //! The user will not be able to modify the object. The object of type T will always be read-only. It is safe!
        return reinterpret_cast<T*>(
            const_cast<uint8_t*>(m_buffer.data())
        );
    } else {
        throw std::runtime_error(getFullClassName() + "::" + __FUNCTION__ +
            ": Attempt to access to empty object");
    }
}

template<typename T>
void LazyConstructedObject<T>::destruct()
{
    std::destroy_at<T>(reinterpret_cast<T*>(get()));
    m_isEmpty = true;
}

template<typename T>
template<typename F>
void LazyConstructedObject<T>::destructNoThrow(F throwHandler)
{
    if (!isEmpty()) {
        std::destroy_at<T>(reinterpret_cast<T*>(get()));
        m_isEmpty = true;
    } else {
        throwHandler();
    }
}

template<typename T>
T LazyConstructedObject<T>::into_value() &
{
    return *get();
}

template<typename T>
T LazyConstructedObject<T>::into_value() const &
{
    return *get();
}

template<typename T>
T LazyConstructedObject<T>::into_value() &&
{
    return *get();
}

template<typename T>
bool LazyConstructedObject<T>::isEmpty() const
{
    return m_isEmpty;
}

template<typename T>
LazyConstructedObject<T>::operator bool() const
{
    return !isEmpty();
}

template<typename T>
const std::string& LazyConstructedObject<T>::getFullClassName() const
{
    static const std::string name = typeid(LazyConstructedObject<T>()).name();
    return name;
}

template<typename T>
template<typename ...Args>
void LazyConstructedObject<T>::operator()(Args&& ...args)
{
    construct(std::forward<Args>(args) ...);
}

template<typename T>
template<typename ...Args>
void LazyConstructedObject<T>::construct(Args&& ...args)
{
    m_isEmpty = false;
    new(get()) T(std::forward<Args>(args) ... );
}

} //! namespace posnet::utils


#endif //! VS_LAZY_H
