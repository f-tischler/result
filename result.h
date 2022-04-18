//
// Created by flori on 03.04.2022.
//

#ifndef ERRORHANDLING_RESULT_H
#define ERRORHANDLING_RESULT_H

#include <functional>
#include <gsl/assert>

#include "storage.h"
#include "error.h"

#include "common_errors.h"

#if defined(__clang__) || defined(__GNUC__)
#define ERR_LIKELY(x) __builtin_expect(!!(x), 1)
#define ERR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define GSL_LIKELY(x) (!!(x))
#define GSL_UNLIKELY(x) (!!(x))
#endif // defined(__clang__) || defined(__GNUC__)

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
        throw std::logic_error(fmt::format("{}", e));

    return { .error = std::move(e) };
}

struct default_final_action
{
    template<class T>
    constexpr void operator()(const T&) const { };
} g_default_final_action;

template<class T, class R>
constexpr inline bool is_final_action_v =
    std::is_invocable_v<T, const R&> &&
    std::is_class_v<T> &&
    std::is_default_constructible_v<T>;

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

template<class Value = void, class Error = error, class FinalAction = detail::default_final_action>
class result;

template<class Value, class Error, class FinalAction>
class [[nodiscard]] result
{
public:
    static_assert(detail::is_final_action_v<FinalAction, result>,
                  "final action must be invocable, an object and default constructible");

    using result_storage = detail::result_storage<Value, Error>;
    using member_storage = std::tuple<result_storage, FinalAction>; // use tuple for empty-baseclass-optimization

    result(result&&) noexcept = default;

    result(detail::failed_result<Error>&& e) // NOLINT(google-explicit-constructor)
        : m_data(result_storage(std::move(e.error)), FinalAction{})
    {
    }

    result(detail::ok_result<Value>&& e) // NOLINT(google-explicit-constructor)
        : m_data(result_storage(std::move(e.value)), FinalAction{})
    {
    }

    [[nodiscard]] finline bool is_ok() const { return get_storage().has_value(); }
    [[nodiscard]] finline bool has_failed() const { return get_storage().has_error(); }
    [[nodiscard]] finline auto get_error() const & -> const Error& { Expects(has_failed()); return get_storage().get_error(); }
    [[nodiscard]] finline auto get_value() const & -> const Value& { Expects(is_ok()); return get_storage().get_value(); }
    [[nodiscard]] finline auto get_error() && -> Error&& { Expects(has_failed()); return std::move(get_storage()).get_error(); }
    [[nodiscard]] finline auto get_value() && -> Value&& { Expects(is_ok()); return std::move(get_storage()).get_value(); }
    [[nodiscard]] finline explicit operator bool() const { return is_ok(); }

//    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
//    [[nodiscard]] auto handle_error(F func) & -> decltype(std::invoke(func, get_error()))
//    {
//        if(is_ok())
//        {
//            return std::move(*this);
//        }
//
//        return std::invoke(func, get_error());
//    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
    [[nodiscard]] result handle_error(F func) // -> decltype(std::invoke(func, get_error()))
    {
        static_assert(std::is_same_v<std::invoke_result_t<F, Error>, result>,
                "error handler must return the same result type");

        if(is_ok())
        {
            return std::move(*this);
        }

        auto r = std::invoke(func, get_error());
        if(r.is_ok())
        {
            return std::move(r);
        }

        return detail::error(std::move(*this).propagate_error(
                                r.get_error().get_code(),
                                r.get_error().get_explanation(),
                                r.get_error().get_origin()));
    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Value>>>
    [[nodiscard]] auto map_value(F func) & -> result<decltype(std::invoke(func, get_value())), Error>
    {
        if(has_failed())
        {
            return std::move(*this);
        }

        return ok(std::invoke(func, get_value()));
    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Value>>>
    [[nodiscard]] auto map_value(F func) && -> result<decltype(std::invoke(func, get_value())), Error>
    {
        if(has_failed())
        {
            return std::move(*this);
        }

        return ok(std::invoke(func, get_value()));
    }

    finline void ignore() const { }

    Error propagate_error(const error_code& ec, std::string_view explanation, source_location origin) &&
    {
        return Error(ec,
                     std::string(explanation),
                     std::make_unique<Error>(std::move(get_storage()).get_error()),
                     origin);
    }

    ~result()
    {
        std::invoke(get_final_action(), *this);
    }

private:
    [[nodiscard]] auto get_storage() -> result_storage& { return std::get<0>(m_data); }
    [[nodiscard]] auto get_storage() const -> const result_storage& { return std::get<0>(m_data); }
    [[nodiscard]] auto get_final_action() -> FinalAction& { return std::get<1>(m_data); }

    member_storage m_data;
};

// sbo
static_assert(sizeof(result<char, error>) == sizeof(detail::result_storage<char, error>),
              "result<T> with default final action must only occupy memory to store the error/value");

// heap
static_assert(sizeof(result<char[sizeof(error) + 1], error>) == sizeof(detail::result_storage<char[sizeof(error) + 1], error>),
              "result<T> with default final action must only occupy memory to store the error/value");

template<class Error, class FinalAction>
class [[nodiscard]] result<void, Error, FinalAction> {
public:
    static_assert(detail::is_final_action_v<FinalAction, result>,
                  "final action must be invocable, an object and default constructible");

    using error_storage = detail::pointer_storage<Error>;
    using member_storage = std::tuple<error_storage, FinalAction>; // use tuple for empty-baseclass-optimization

    result() = default;

    result(result&&) noexcept = default;

    result(detail::ok_result<> &&) // NOLINT(google-explicit-constructor)
    {
    }

    result(detail::failed_result<Error> &&e) // NOLINT(google-explicit-constructor)
        : m_data(error_storage(std::move(e.error)), FinalAction{})
    {
    }

    [[nodiscard]] finline bool is_ok() const { return !get_error_storage().has_value(); }
    [[nodiscard]] finline bool has_failed() const { return get_error_storage().has_value(); }
    [[nodiscard]] finline auto get_error() const & -> const Error& { Expects(has_failed()); return get_error_storage().get(); }
    [[nodiscard]] finline auto get_error() && -> Error&& { Expects(has_failed()); return std::move(get_error_storage()).get(); }
    [[nodiscard]] finline explicit operator bool() const { return is_ok(); }

//    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
//    [[nodiscard]] auto handle_error(F func) & -> decltype(std::invoke(func, get_error()))
//    {
//        if(is_ok())
//        {
//            return std::move(*this);
//        }
//
//        return std::invoke(func, get_error());
//    }

    template<class F, typename = std::enable_if_t<std::is_invocable_v<F, Error>>>
    [[nodiscard]] result handle_error(F func) // -> decltype(std::invoke(func, get_error()))
    {
        if(is_ok())
        {
            return std::move(*this);
        }

        auto r = std::invoke(func, get_error());
        if(r.is_ok())
        {
            return std::move(r);
        }

        return detail::error(std::move(*this).propagate_error(
                r.get_error().get_code(),
                r.get_error().get_explanation(),
                r.get_error().get_origin()));
    }

    // will NOT suppress call of final action
    //    finline void ignore() const { }

    // will suppress call of final action
    finline void dismiss() { get_error_storage().reset(); }

    Error propagate_error(const error_code& ec, std::string_view explanation, source_location origin) &&
    {
        return Error(ec,
                     std::string(explanation),
                     std::unique_ptr<Error>(get_error_storage().release()),
                     origin);
    }

    ~result()
    {
        std::invoke(get_final_action(), *this);
    }

private:
    [[nodiscard]] auto get_error_storage() -> error_storage& { return std::get<0>(m_data); }
    [[nodiscard]] auto get_error_storage() const -> const error_storage& { return std::get<0>(m_data); }
    [[nodiscard]] auto get_final_action() const -> const FinalAction& { return std::get<1>(m_data); }

    member_storage m_data;
};

static_assert(sizeof(result<void, error>) == sizeof(detail::pointer_storage<error>),
              "result with default final action must only occupy memory to store the error");

template<class V, class E, class F>
std::ostream& operator<<(std::ostream& os, const result<V, E, F>& result)
{
    return os << result.get_error().to_string();
}

#endif //ERRORHANDLING_RESULT_H
