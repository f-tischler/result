//
// Created by flori on 03.04.2022.
//

#ifndef ERRORHANDLING_RESULT_H
#define ERRORHANDLING_RESULT_H

#include <functional>

#include "storage.h"
#include "error.h"

#include "common_errors.h"

namespace detail
{

template<class T = void>
struct [[nodiscard]] ok_result
{
    T value;
};

template<>
struct [[nodiscard]] ok_result<void>
{
};

template<class T>
struct [[nodiscard]] failed_result
{
    T error;
};

template<class Error = error>
detail::failed_result<std::decay_t<Error>> error(Error&& error)
{
    return { .error = std::forward<Error>(error) };
}

template<class ErrorCode, class Error = class error>
detail::failed_result<std::decay_t<Error>> error(ErrorCode code, std::string explanation, source_location src_loc)
{
    Error e{ code, explanation, src_loc };

    if(code.get_category() == assertion_errors::assertion_category{})
        throw std::logic_error(e.to_string());

    return { .error = std::move(e) };
}

}

detail::ok_result<> ok()
{
    return {};
}

template<class Value, typename std::enable_if_t<!std::is_lvalue_reference_v<Value>>* = nullptr>
detail::ok_result<std::remove_const_t<Value>> ok(Value value)
{
    return { .value = std::move(value) };
}

template<class Value = void, class Error = error>
class result;

template<class Value, class Error>
class [[nodiscard]] result
{
public:
    result(result&&) = default;

    result(detail::failed_result<Error>&& e) // NOLINT(google-explicit-constructor)
        : m_result(std::move(e.error))
    {
    }

    result(detail::ok_result<Value>&& e) // NOLINT(google-explicit-constructor)
        : m_result(std::move(e.value))
    {
    }

    [[nodiscard]] finline bool is_ok() const { return m_result.has_value(); }
    [[nodiscard]] finline bool has_failed() const { return !is_ok(); }
    [[nodiscard]] finline auto get_error() const -> const Error& { return m_result.get_error(); }
    [[nodiscard]] finline auto get_value() const -> const Value& { return m_result.get_value(); }
    [[nodiscard]] finline explicit operator bool() const { return is_ok(); }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
    [[nodiscard]] auto handle_error(F func) & -> decltype(std::invoke(func, get_error()))
    {
        if(is_ok())
        {
            return std::move(*this);
        }

        return std::invoke(func, get_error());
    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
    [[nodiscard]] auto handle_error(F func) && -> decltype(std::invoke(func, get_error()))
    {
        if(is_ok())
        {
            return std::move(*this);
        }

        return std::invoke(func, get_error());
    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Value>>>
    [[nodiscard]] auto map(F func) & -> result<decltype(std::invoke(func, get_value())), Error>
    {
        if(has_failed())
        {
            return detail::error(get_error());
        }

        return ok(std::invoke(func, get_value()));
    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Value>>>
    [[nodiscard]] auto map(F func) && -> result<decltype(std::invoke(func, get_value())), Error>
    {
        if(has_failed())
        {
            return detail::error(get_error());
        }

        return ok(std::invoke(func, get_value()));
    }

    finline void ignore() const { }

private:
    detail::result_storage<Value, Error> m_result;
};

template<class Error>
class [[nodiscard]] result<void, Error>
{
public:
    result() = default;
    result(result&&) noexcept = default;

    result(detail::ok_result<>&&) // NOLINT(google-explicit-constructor)
    {
    }

    result(detail::failed_result<Error>&& e) // NOLINT(google-explicit-constructor)
        : m_error(std::move(e.error))
    {
    }

    [[nodiscard]] finline bool is_ok() const { return !m_error.has_value(); }
    [[nodiscard]] finline bool has_failed() const { return !is_ok(); }
    [[nodiscard]] finline auto get_error() const -> const Error& { return m_error.get(); }
    [[nodiscard]] finline explicit operator bool() const { return is_ok(); }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
    [[nodiscard]] auto handle_error(F func) & -> decltype(std::invoke(func, get_error()))
    {
        if(is_ok())
        {
            return std::move(*this);
        }

        return std::invoke(func, get_error());
    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
    [[nodiscard]] auto handle_error(F func) && -> decltype(std::invoke(func, get_error()))
    {
        if(is_ok())
        {
            return std::move(*this);
        }

        return std::invoke(func, get_error());
    }

    finline void ignore() const { }

private:
    detail::pointer_storage<Error> m_error;
};

#endif //ERRORHANDLING_RESULT_H
