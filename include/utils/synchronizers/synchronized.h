#pragma once

#include <cassert>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>

// Lightweight version of folly::Synchronized.

namespace posnet::utils {

namespace detail {

template <class T, class = void>
struct is_shared_mutex : std::false_type {};
template <class T>
struct is_shared_mutex<T, std::void_t<decltype(std::declval<T&>().lock_shared(), std::declval<T&>().unlock_shared())>> : std::true_type {};

struct ExclusiveLockPolicy
{
	template <class Mutex> static void lock  (Mutex& m) { m.lock(); }
	template <class Mutex> static void unlock(Mutex& m) { m.unlock(); }
};

struct SharedLockPolicy
{
	template <class Mutex> static void lock  (Mutex& m) { m.lock_shared(); }
	template <class Mutex> static void unlock(Mutex& m) { m.unlock_shared(); }
};

template <class S, class LockPolicy> class LockedPtr;

}

template <class T, class Mutex = std::mutex>
class Synchronized
{
public:
	using value_type = T;
	using mutex_type = Mutex;

	using LockedPtr = detail::LockedPtr<Synchronized, detail::ExclusiveLockPolicy>;
	using ConstLockedPtr = detail::LockedPtr<const Synchronized, detail::ExclusiveLockPolicy>;
	using SharedLockedPtr = detail::LockedPtr<const Synchronized, detail::SharedLockPolicy>;

	template <class... Args>
	explicit Synchronized(Args&&... args)
		: m_data(std::forward<Args>(args)...)
	{
	}

	Synchronized(const Synchronized& x)
		: m_data(*x.lock())
		, m_mutex()
	{
	}

	Synchronized(Synchronized&& x)
		: m_data(std::move(*x.lock()))
		, m_mutex()
	{
	}

	Synchronized& operator=(const Synchronized& x)
	{
		if (this == &x)
			return *this;

		// Global ordering of locks to avoid deadlock
		if (this < &x)
		{
			std::lock_guard<Mutex> l(m_mutex);
			std::lock_guard<Mutex> l_x(x.m_mutex);
			m_data = x.m_data;
		}
		else
		{
			std::lock_guard<Mutex> l_x(x.m_mutex);
			std::lock_guard<Mutex> l(m_mutex);
			m_data = x.m_data;
		}
		return *this;
	}

	Synchronized& operator=(Synchronized&& x)
	{
		if (this == &x)
			return *this;

		// Global ordering of locks to avoid deadlock
		if (this < &x)
		{
			std::lock_guard<Mutex> l(m_mutex);
			std::lock_guard<Mutex> l_x(x.m_mutex);
			m_data = std::move(x.m_data);
		}
		else
		{
			std::lock_guard<Mutex> l_x(x.m_mutex);
			std::lock_guard<Mutex> l(m_mutex);
			m_data = std::move(x.m_data);
		}
		return *this;
	}

	Synchronized& operator=(const T& x)
	{
		std::lock_guard<Mutex> l(m_mutex);
		m_data = x;
		return *this;
	}

	Synchronized& operator=(T&& x)
	{
		std::lock_guard<Mutex> l(m_mutex);
		m_data = std::move(x);
		return *this;
	}

	void swap(Synchronized& x)
	{
		if (this == &x)
			return;

		using std::swap;
		// Global ordering of locks to avoid deadlock
		if (this < &x)
		{
			std::lock_guard<Mutex> l(m_mutex);
			std::lock_guard<Mutex> l_x(x.m_mutex);
			swap(m_data, x.m_data);
		}
		else
		{
			std::lock_guard<Mutex> l_x(x.m_mutex);
			std::lock_guard<Mutex> l(m_mutex);
			swap(m_data, x.m_data);
		}
	}

	void swap(T& x)
	{
		using std::swap;
		std::lock_guard<Mutex> l(m_mutex);
		swap(m_data, x);
	}

	LockedPtr lock()
	{
		return { this };
	}

	ConstLockedPtr lock() const
	{
		return { this };
	}

	SharedLockedPtr sharedLock() const
	{
		return { this };
	}

	LockedPtr genericLock()
	{
		return { this };
	}

	auto genericLock() const
	{
		if constexpr (detail::is_shared_mutex<T>::value)
			return SharedLockedPtr(this);
		else
			return ConstLockedPtr(this);
	}

	LockedPtr operator->()
	{
		return { this };
	}

	ConstLockedPtr operator->() const
	{
		return { this };
	}

	template <class F>
	auto withLock(F&& f)
	{
		std::lock_guard<Mutex> l(m_mutex);
		return std::forward<F>(f)(m_data);
	}

	template <class F>
	auto withLock(F&& f) const
	{
		std::lock_guard<Mutex> l(m_mutex);
		return std::forward<F>(f)(m_data);
	}

	template <class F>
	auto withSharedLock(F&& f) const
	{
		std::shared_lock<Mutex> l(m_mutex);
		return std::forward<F>(f)(m_data);
	}

	template <class F>
	auto withGenericLock(F&& f)
	{
		if constexpr (detail::is_shared_mutex<T>::value && std::is_invocable_v<F&&, const T&>)
		{
			std::shared_lock<Mutex> l(m_mutex);
			return std::forward<F>(f)(std::as_const(m_data));
		}
		else
		{
			std::lock_guard<Mutex> l(m_mutex);
			return std::forward<F>(f)(m_data);
		}
	}

	template <class F>
	auto withGenericLock(F&& f) const
	{
		if constexpr (detail::is_shared_mutex<T>::value)
		{
			std::shared_lock<Mutex> l(m_mutex);
			return std::forward<F>(f)(m_data);
		}
		else
		{
			std::lock_guard<Mutex> l(m_mutex);
			return std::forward<F>(f)(m_data);
		}
	}

	template <class F>
	auto withLockPtr(F&& f)
	{
		return std::forward<F>(f)(LockedPtr(this));
	}

	template <class F>
	auto withLockPtr(F&& f) const
	{
		return std::forward<F>(f)(ConstLockedPtr(this));
	}

	template <class F>
	auto withSharedLockPtr(F&& f) const
	{
		return std::forward<F>(f)(SharedLockedPtr(this));
	}

	template <class F>
	auto withGenericLockPtr(F&& f)
	{
		return std::forward<F>(f)(LockedPtr(this));
	}

	template <class F>
	auto withGenericLockPtr(F&& f) const
	{
		if constexpr (detail::is_shared_mutex<T>::value)
			return std::forward<F>(f)(SharedLockedPtr(this));
		else
			return std::forward<F>(f)(ConstLockedPtr(this));
	}

private:
	T m_data;
	mutable Mutex m_mutex;

	friend LockedPtr;
	friend ConstLockedPtr;
	friend SharedLockedPtr;

};

namespace detail {

template <class S, class LockPolicy>
class LockedPtr
{
	class UnlockGuard
	{
	public:
		explicit UnlockGuard(LockedPtr* ptr)
			: m_ptr(ptr)
			, m_saved_parent(m_ptr->m_parent)
		{
			assert(m_ptr->m_parent);
			LockPolicy::unlock(m_ptr->m_parent->m_mutex);
			m_ptr->m_parent = nullptr;
		}
		UnlockGuard(const UnlockGuard&) = delete;
		UnlockGuard(UnlockGuard&& x) noexcept
			: m_ptr(x.m_ptr)
			, m_saved_parent(x.m_saved_parent)
		{
			x.m_ptr = nullptr;
			x.m_saved_parent = nullptr;
		}
		UnlockGuard& operator=(const UnlockGuard&) = delete;
		UnlockGuard& operator=(UnlockGuard&& x) = delete;

		~UnlockGuard()
		{
			if (m_ptr)
			{
				m_ptr->m_parent = m_saved_parent;
				LockPolicy::lock(m_ptr->m_parent->m_mutex);
			}
		}

	private:
		LockedPtr* m_ptr;
		S* m_saved_parent;
	};

public:
	// cppcheck-suppress noExplicitConstructor
	LockedPtr(S* parent)
		: m_parent(parent)
	{
		assert(m_parent);
		LockPolicy::lock(m_parent->m_mutex);
	}

	LockedPtr(const LockedPtr&) = delete;
	LockedPtr(LockedPtr&& x) noexcept
		: m_parent(x.m_parent)
	{
		x.m_parent = nullptr;
	}

	LockedPtr& operator=(const LockedPtr&) = delete;
	LockedPtr& operator=(LockedPtr&& x) noexcept
	{
		if (this == &x)
			return *this;
		if (m_parent)
			LockPolicy::unlock(m_parent->m_mutex);
		m_parent = x.m_parent;
		x.m_parent = nullptr;
		return *this;
	}

	~LockedPtr()
	{
		if (m_parent)
			LockPolicy::unlock(m_parent->m_mutex);
	}

	explicit operator bool() const noexcept
	{
		return m_parent != nullptr;
	}

	decltype(&std::declval<S&>().m_data) operator->() const noexcept
	{
		assert(m_parent);
		return &m_parent->m_data;
	}

	// Note: Don't remove extra parenthesis on the next line, they are needed to force decltype to threat its argument as an expression.
	decltype((std::declval<S&>().m_data)) operator*() const noexcept
	{
		assert(m_parent);
		return m_parent->m_data;
	}

	UnlockGuard scopedUnlock()
	{
		return UnlockGuard(this);
	}

private:
	S* m_parent;
};

}

} //! namespace posnet::utils
