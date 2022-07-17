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
#include "assert.h"
#include "types.h"

namespace detail
{
    template<class T>
    struct with_default_final_action;

    template<class Value, class Error, class FinalAction>
    struct with_default_final_action<result<Value, Error, FinalAction>>
    {
        using type = result<Value, Error, detail::default_final_action>;
    };

    template<class T>
    using with_default_final_action_t = typename with_default_final_action<T>::type;

    template<class Value, class Error, class FinalAction, class F>
    auto handle_error(result<Value, Error, FinalAction>&& inner, F&& handler) -> result<Value, Error, FinalAction>
    {
        static_assert(std::is_same_v<with_default_final_action_t<std::invoke_result_t<F, Error>>,
                                     with_default_final_action_t<result<Value, Error, FinalAction>>>,
                      "error handler must return the same result type");

        if(inner.is_ok())
        {
            return std::move(inner);
        }

        auto outer = std::invoke(std::forward<F>(handler), inner.get_error());
        if(outer.is_ok())
        {
            return std::move(outer);
        }

        return detail::make_failure(std::move(outer).release_error(),
                                    std::move(inner).release_error());
    }
}

template<class Value, class Error, class FinalAction>
class [[nodiscard]] result
{
public:
    static_assert(detail::is_final_action_v<FinalAction, result>,
                  "final action must be invocable and default constructible");

    using result_storage = detail::result_storage<Value, Error>;
    using member_storage = std::tuple<result_storage, FinalAction>; // use tuple for empty-baseclass-optimization

    result(result&&) noexcept = default;

    template<class V, class E, class F, typename = std::enable_if_t<!std::is_same_v<F, FinalAction>>>
    result(result<V, E, F>&& r) noexcept
        : m_data(result_storage(result_storage(std::move(r).release_error()), FinalAction{}))
    {
    }

    result(detail::failure<Error>&& e) // NOLINT(google-explicit-constructor)
        : m_data(result_storage(std::move(e.error)), FinalAction{})
    {
    }

    result(detail::success<Value>&& e) // NOLINT(google-explicit-constructor)
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
    [[nodiscard]] result handle_error(F&& handler)
    {
        return detail::handle_error(std::move(*this), std::forward<F>(handler));
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

    auto release_error() && -> std::unique_ptr<Error>
    {
        return std::make_unique<Error>(std::move(get_storage()).get_error());
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
              "result<T> with default final action must only occupy memory to store the make_failure/value");

// heap
static_assert(sizeof(result<char[sizeof(error) + 1], error>) == sizeof(detail::result_storage<char[sizeof(error) + 1], error>),
              "result<T> with default final action must only occupy memory to store the make_failure/value");

template<class Error, class FinalAction>
class [[nodiscard]] result<void, Error, FinalAction> {
public:
    static_assert(detail::is_final_action_v<FinalAction, result>,
                  "final action must be invocable and default constructible");

    using error_storage = detail::pointer_storage<Error>;
    using member_storage = std::tuple<error_storage, FinalAction>; // use tuple for empty-baseclass-optimization

    result() = default;

    result(result&&) noexcept = default;

    template<class E, class F, typename = std::enable_if_t<!std::is_same_v<F, FinalAction>>>
    result(result<void, E, F>&& r) noexcept // NOLINT(google-explicit-constructor)
        : m_data(error_storage(std::move(r).release_error()), FinalAction{})
    {
    }

    result(detail::success<> &&) // NOLINT(google-explicit-constructor)
    {
    }

    result(detail::failure<Error> &&e) // NOLINT(google-explicit-constructor)
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
    [[nodiscard]] result handle_error(F&& handler) // -> decltype(std::invoke(func, get_error()))
    {
        return detail::handle_error(std::move(*this), std::forward<F>(handler));
    }

    // will NOT suppress call of final action
    //    finline void ignore() const { }

    // will suppress call of final action
    finline void dismiss() { get_error_storage().reset(); }

    auto release_error() && -> std::unique_ptr<Error>
    {
        return std::unique_ptr<Error>(get_error_storage().release());
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
