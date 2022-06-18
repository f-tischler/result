//
// Created by flori on 09.04.2022.
//

#ifndef ERRORHANDLING_ASSERT_H
#define ERRORHANDLING_ASSERT_H

#include "types.h"
#include "common_errors.h"
#include "make_result.h"

#include <fmt/core.h>

template<class T>
std::string format_expression(std::string_view expr, const T& result, std::string_view explanation)
{
    return fmt::format("Expression: '{}'\n"
                       "Result:      {}\n"
                       "Explanation: {}",
                       expr, result, explanation);
}

template<class V, class E, class L>
auto fail_precondition(result<V, E, L>&& result,
                       std::string_view expr,
                       std::string_view explanation,
                       source_location origin)
{
    return detail::make_failed_result(assertion_errors::precondition_error{},
                                      format_expression(expr, result, explanation),
                                      std::move(result).release_error(),
                                      origin);
}

template<class T>
auto fail_precondition(T&& result, std::string_view expr, std::string_view explanation, source_location origin)
{
    return detail::make_failed_result(assertion_errors::precondition_error{},
                                      format_expression(expr, result, explanation),
                                      origin);
}

template<class V, class E, class L>
auto fail_postcondition(result<V, E, L>&& result, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::make_failed_result(assertion_errors::postcondition_error{},
                                      format_expression(expr, result, explanation),
                                      std::move(result).release_error(),
                                      src_loc);
}

template<class T>
auto fail_postcondition(T&& result, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::make_failed_result(assertion_errors::postcondition_error{},
                                      format_expression(expr, result, explanation),
                                      src_loc);
}

#endif //ERRORHANDLING_ASSERT_H
