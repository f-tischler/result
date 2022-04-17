//
// Created by flori on 09.04.2022.
//

#ifndef ERRORHANDLING_ASSERT_H
#define ERRORHANDLING_ASSERT_H

#include "common_errors.h"

template<class V, class E, class L>
class result;

template<class T>
std::string format_expression(std::string_view expr, const T& result, std::string_view explanation)
{
    return fmt::format("Expression: '{}'\n"
                       "Result:      {}\n"
                       "Explanation: {}",
                       expr, result, explanation);
}

template<class V, class E>
auto fail_precondition(result<V, E>&& result, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::precondition_error{},
                         format_expression(expr, result, explanation),
                         src_loc);
}

template<class T>
auto fail_precondition(T&& result, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::precondition_error{},
                         format_expression(expr, result, explanation),
                         src_loc);
}

template<class V, class E>
auto fail_postcondition(result<V, E>&& result, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::postcondition_error{},
                         format_expression(expr, result, explanation),
                         src_loc);
}

template<class T>
auto fail_postcondition(T&& result, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::postcondition_error{},
                         format_expression(expr, result, explanation),
                         src_loc);
}

#endif //ERRORHANDLING_ASSERT_H
