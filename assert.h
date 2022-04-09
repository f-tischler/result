//
// Created by flori on 09.04.2022.
//

#ifndef ERRORHANDLING_ASSERT_H
#define ERRORHANDLING_ASSERT_H

#include "common_errors.h"

template<class V, class E>
auto fail_precondition(result<V, E>&&, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::precondition_error{},
                         fmt::format("Expression '{}'. {}", expr, explanation),
                         src_loc);
}

template<class T>
auto fail_precondition(T&&, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::precondition_error{},
                         fmt::format("Expression '{}'. {}", expr, explanation),
                         src_loc);
}

template<class V, class E>
auto fail_postcondition(result<V, E>&&, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::postcondition_error{},
                         fmt::format("Expression '{}'. {}", expr, explanation),
                         src_loc);
}

template<class T>
auto fail_postcondition(T&&, std::string_view expr, std::string_view explanation, source_location src_loc)
{
    return detail::error(assertion_errors::postcondition_error{},
                         fmt::format("Expression '{}'. {}", expr, explanation),
                         src_loc);
}

#endif //ERRORHANDLING_ASSERT_H
