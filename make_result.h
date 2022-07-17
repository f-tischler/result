//
// Created by flori on 23.04.2022.
//

#ifndef ERRORHANDLING_MAKE_RESULT_H
#define ERRORHANDLING_MAKE_RESULT_H

#include "error.h"
#include "assert.h"

namespace detail
{
    template<class T = void>
    struct [[nodiscard]] success;

    template<>
    struct [[nodiscard]] success<void> {};

    template<class T>
    struct [[nodiscard]] success
    {
        T value;
    };

    template<class T>
    struct [[nodiscard]] failure
    {
        T error;
    };

    template<class Error = error>
    failure<std::decay_t<Error>> make_failure(Error&& error)
    {
        return { .error = std::forward<Error>(error) };
    }

    template<class ErrorCode, class Error = class error>
    failure<std::decay_t<Error>> make_failure(ErrorCode&& code,
                                              std::string explanation,
                                              source_location src_loc)
    {
        return
        {
            .error = Error(std::forward<ErrorCode&&>(code),
                           std::move(explanation),
                           src_loc)
        };
    }

    template<class ErrorCode, class Error = class error>
    failure<std::decay_t<Error>> make_failure(ErrorCode code,
                                              std::string explanation,
                                              std::unique_ptr<Error> innerError,
                                              source_location src_loc)
    {
        return
        {
            .error = Error(std::forward<ErrorCode&&>(code),
                           std::move(explanation),
                           std::move(innerError),
                           src_loc)
        };
    }

    template<class ErrorCode, class T, class Error = class error>
    failure<std::decay_t<Error>> make_failure(ErrorCode code,
                                              std::string explanation,
                                              T&& data,
                                              source_location src_loc)
    {
        return
        {
            .error = std::move(Error(std::forward<ErrorCode&&>(code),
                           std::move(explanation),
                           src_loc).set_data(std::forward<T>(data)))
        };
    }

    template<class ErrorCode, class T, class Error = class error>
    failure<std::decay_t<Error>> make_failure(ErrorCode code,
                                              std::string explanation,
                                              std::unique_ptr<Error> innerError,
                                              T&& data,
                                              source_location src_loc)
    {
        return
        {
            .error = Error(std::forward<ErrorCode&&>(code),
                           std::move(explanation),
                           std::move(innerError),
                           src_loc).set_data(std::forward<T>(data))

        };
    }

    template<class Error = class error>
    failure<std::decay_t<Error>> make_failure(std::unique_ptr<Error>&& outerError,
                                              std::unique_ptr<Error>&& innerError)
    {
        return
        {
            .error = std::move(outerError->set_inner_error(std::move(innerError)))
        };
    }
}

detail::success<> ok()
{
    return {};
}

template<class Value, typename std::enable_if_t<!std::is_lvalue_reference_v<Value>> * = nullptr>
detail::success<std::remove_const_t<Value>> ok(Value value)
{
    return { .value = std::move(value) };
}

#endif //ERRORHANDLING_MAKE_RESULT_H
