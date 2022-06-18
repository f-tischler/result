//
// Created by flori on 23.04.2022.
//

#ifndef ERRORHANDLING_MAKE_RESULT_H
#define ERRORHANDLING_MAKE_RESULT_H

#include "error.h"
#include "assert.h"

class AssertionException
        : public std::logic_error
{
public:
    explicit AssertionException(error&& e)
            : std::logic_error("Assertion make_failed_result")
            , m_error(std::move(e))
    {

    }

    [[nodiscard]] const error& get_error() const { return m_error; }

private:
    error m_error;
};

namespace detail
{
    template<class T = void>
    struct [[nodiscard]] ok_result;

    template<>
    struct [[nodiscard]] ok_result<void> {};

    template<class T>
    struct [[nodiscard]] ok_result
    {
        T value;
    };

    template<class T>
    struct [[nodiscard]] failed_result
    {
        T error;
    };

    template<class Error = error>
    failed_result<std::decay_t<Error>> make_failed_result(Error&& error)
    {
        return { .error = std::forward<Error>(error) };
    }

    template<class ErrorCode, class Error = class error>
    failed_result<std::decay_t<Error>> make_failed_result(ErrorCode&& code,
                                                          std::string explanation,
                                                          source_location src_loc)
    {
        Error e{std::forward<ErrorCode&&>(code), std::move(explanation), src_loc};

        if (code.get_category() == assertion_errors::assertion_category{})
            throw AssertionException(std::move(e));

        return { .error = std::move(e) };
    }

    template<class ErrorCode, class Error = class error>
    failed_result<std::decay_t<Error>> make_failed_result(ErrorCode code,
                                                          std::string explanation,
                                                          std::unique_ptr<Error> innerError,
                                                          source_location src_loc)
    {
        Error e{std::forward<ErrorCode&&>(code), std::move(explanation), std::move(innerError), src_loc};

        if (code.get_category() == assertion_errors::assertion_category{})
            throw AssertionException(std::move(e));

        return { .error = std::move(e) };
    }

    template<class ErrorCode, class T, class Error = class error>
    failed_result<std::decay_t<Error>> make_failed_result(ErrorCode code,
                                                          std::string explanation,
                                                          T&& data,
                                                          source_location src_loc)
    {
        Error e{std::forward<ErrorCode&&>(code), std::move(explanation), src_loc};
        e.set_data(std::forward<T>(data));

        if (code.get_category() == assertion_errors::assertion_category{})
            throw AssertionException(std::move(e));

        return { .error = std::move(e) };
    }

    template<class ErrorCode, class T, class Error = class error>
    failed_result<std::decay_t<Error>> make_failed_result(ErrorCode code,
                                                          std::string explanation,
                                                          std::unique_ptr<Error> innerError,
                                                          T&& data,
                                                          source_location src_loc)
    {
        Error e{std::forward<ErrorCode&&>(code), std::move(explanation), std::move(innerError), src_loc};
        e.set_data(std::forward<T>(data));

        if (code.get_category() == assertion_errors::assertion_category{})
            throw AssertionException(std::move(e));

        return { .error = std::move(e) };
    }
}

detail::ok_result<> ok()
{
    return {};
}

template<class Value, typename std::enable_if_t<!std::is_lvalue_reference_v<Value>> * = nullptr>
detail::ok_result<std::remove_const_t<Value>> ok(Value value)
{
    return { .value = std::move(value) };
}

#endif //ERRORHANDLING_MAKE_RESULT_H
