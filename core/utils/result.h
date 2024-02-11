#ifndef VS_RESULT_H
#define VS_RESULT_H

#include <utility>
#include <variant>
#include <optional>
#include <string>

/**
* @brief Result<T, E> is the type used for returning and propagating errors. It is an class with the variants, onOk(T),
*        representing success and containing a value, and onErr(E), representing error and containing an error value.
* @detail This class is not thread safe, it meats that you can't share this class between threads without synchronization primitives.
*         This class can throw exception: std::bad_variant_access.
* @example: Result<int, ErrorCodeType> division(int x, int y) {
*                return y != 0 ?
*                    Result<int, ErrorCodeType>::onOk(x / y) :
*                    Result<int, ErrorCodeType>::onError(ErrorCodeType::DivisionByZeroError);
*            }
*
*            auto result = division(10, 2);
*            if (result) {
*                std::cout << "Division success, result=" << *result << std::endl;
*            } else {
*                std::cout << "Division is failed" << std::endl;
*            }
*/
namespace utils {

template<typename T, typename E>
class [[nodiscard]] Result final {
public:
    using ResultType = T;
    using ErrorType = E;

    ResultType& operator*() && noexcept(false) = delete;
    ResultType* operator->() && noexcept(false) = delete;
    ResultType& value() && noexcept(false) = delete;
    ErrorType& error() && noexcept(false) = delete;

    Result(const Result&) = default;
    Result& operator=(const Result&) = default;
    Result(Result&&) noexcept = default;
    Result& operator=(Result&&) noexcept = default;

    template<typename R>
    static Result onOk(R&& result);

    template<typename R>
    static Result onError(R&& error);

    operator bool() const;

    bool isOk() const;

    bool isError() const;

    ResultType& operator*() & noexcept(false);

    const ResultType& operator*() const& noexcept(false);

    ResultType* operator->() & noexcept(false);

    const ResultType* const operator->() const& noexcept(false);

    ResultType& value() & noexcept(false);

    const ResultType& value() const& noexcept(false);

    ErrorType& error() & noexcept(false);

    const ErrorType& error() const& noexcept(false);

private:
    template<typename R>
    explicit Result(R&& r);

    std::variant<ResultType, ErrorType> m_data;
};

template<typename E>
class Result<void, E> final {
public:
    using ErrorType = E;

    ErrorType& error() && noexcept(false) = delete;

    Result(const Result&) = default;
    Result& operator=(const Result&) = default;
    Result(Result&&) noexcept = default;
    Result& operator=(Result&&) noexcept = default;

    template<typename R>
    static Result onError(R&& error);

    static Result onOk();

    operator bool() const;

    bool isError() const;

    bool isOk() const;

    ErrorType& error() & noexcept(false);

    const ErrorType& error() const& noexcept(false);

private:
    template<typename R>
    explicit Result(R&& error);

    explicit Result();

    std::optional<ErrorType> m_data;
};

template<typename T, typename E>
template<typename R>
Result<T, E>::Result(R&& r) :
    m_data(std::forward<R>(r))
{}

template<typename T, typename E>
template<typename R>
Result<T, E> Result<T, E>::onOk(R&& result) {
    return Result(std::forward<R>(result));
}

template<typename T, typename E>
template<typename R>
Result<T, E> Result<T, E>::onError(R&& error) {
    return Result(std::forward<R>(error));
}

template<typename T, typename E>
Result<T, E>::operator bool() const {
    return isOk();
}

template<typename T, typename E>
bool Result<T, E>::isOk() const {
    return std::holds_alternative<ResultType>(m_data);
}

template<typename T, typename E>
bool Result<T, E>::isError() const {
    return !isOk();
}

template<typename T, typename E>
typename Result<T, E>::ResultType& Result<T, E>::operator*()& {
    return value();
}

template<typename T, typename E>
const typename Result<T, E>::ResultType& Result<T, E>::operator*() const& {
    return value();
}

template<typename T, typename E>
typename Result<T, E>::ResultType* Result<T, E>::operator->()& {
    return &(value());
}

template<typename T, typename E>
const typename Result<T, E>::ResultType* const Result<T, E>::operator->() const& {
    return &(value());
}

template<typename T, typename E>
typename Result<T, E>::ResultType& Result<T, E>::value()& {
    return std::get<ResultType>(m_data);
}

template<typename T, typename E>
const typename Result<T, E>::ResultType& Result<T, E>::value() const& {
    return std::get<ResultType>(m_data);
}

template<typename T, typename E>
typename Result<T, E>::ErrorType& Result<T, E>::error()& {
    return std::get<ErrorType>(m_data);
}

template<typename T, typename E>
const typename Result<T, E>::ErrorType& Result<T, E>::error() const& {
    return std::get<ErrorType>(m_data);
}

template<typename E>
template<typename R>
Result<void, E>::Result(R&& error) :
    m_data(std::forward<R>(error))
{}

template<typename E>
Result<void, E>::Result() :
    m_data(std::nullopt)
{}

template<typename E>
template<typename R>
Result<void, E> Result<void, E>::onError(R&& error) {
    return Result(std::forward<R>(error));
}

template<typename E>
Result<void, E> Result<void, E>::onOk() {
    return Result();
}

template<typename E>
Result<void, E>::operator bool() const {
    return isOk();
}

template<typename E>
bool Result<void, E>::isError() const {
    return m_data.has_value();
}

template<typename E>
bool Result<void, E>::isOk() const {
    return !isError();
}

template<typename E>
typename Result<void, E>::ErrorType& Result<void, E>::error()& {
    return m_data.value();
}

template<typename E>
const typename Result<void, E>::ErrorType& Result<void, E>::error() const& {
    return m_data.value();
}

template<typename T>
using DefaultResult = Result<T, std::string>;

} // namespace utils

#endif //! VS_RESULT_H
