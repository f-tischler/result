//
// Created by flori on 03.04.2022.
//

#ifndef ERRORHANDLING_MACROS_H
#define ERRORHANDLING_MACROS_H

#include "assert.h"

namespace detail
{

    template<class ErrorCode, class V, class E, class L>
    auto resolve_failed_result(ErrorCode &&code,
                               std::string_view explanation,
                               result<V, E, L> &&result,
                               source_location origin)
    {
        return detail::make_failed_result(std::forward<ErrorCode>(code),
                                          std::string(explanation),
                                          std::move(result).release_error(),
                                          origin);
    }

    template<class ErrorCode, class T>
    auto resolve_failed_result(ErrorCode &&code,
                               std::string_view explanation,
                               T &&data,
                               source_location origin)
    {
        return detail::make_failed_result(std::forward<ErrorCode>(code),
                                          std::string(explanation),
                                          std::forward<T>(data),
                                          origin);
    }

    template<class ErrorCode, class V, class E, class L, class T>
    auto resolve_failed_result(ErrorCode &&code,
                               std::string_view explanation,
                               result <V, E, L> &result,
                               T &&data,
                               source_location origin)
    {
        return detail::make_failed_result(std::forward<ErrorCode>(code),
                                          std::string(explanation),
                                          std::move(result).release_error(),
                                          std::forward<T>(data),
                                          origin);
    }

}

#define CAT( A, B ) A ## B
#define SELECT( NAME, NUM ) CAT( NAME ## _, NUM )

#define GET_COUNT( _1, _2, _3, _4, _5, _6 /* ad nauseam */, COUNT, ... ) COUNT
#define VA_SIZE( ... ) GET_COUNT( __VA_ARGS__, 6, 5, 4, 3, 2, 1 )

#define VA_SELECT( NAME, ... ) SELECT( NAME, VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)

#define TRY_GLUE2(x, y) x##y
#define TRY_GLUE(x, y) TRY_GLUE2(x, y)
#define TRY_UNIQUE_NAME TRY_GLUE(_result_unique_name_temporary, __COUNTER__)

#define TRY_ASSIGN_IMPL(init, result_name, expr) \
    auto result_name = (expr); \
    if(result_name.has_failed()) \
    {                                     \
        return detail::make_failed_result(basic_errors::propagated_error{}, \
                                          #expr, \
                                          std::move(result_name).release_error(), \
                                          { __FILE__, __LINE__ }); \
    } \
    init = std::move(result_name).get_value()

#define TRY_IMPL(result_name, expr) \
    do {                                \
        auto result_name = (expr); \
        if(result_name.has_failed()) \
        {                                     \
            return detail::make_failed_result(basic_errors::propagated_error{}, \
                                              #expr,                            \
                                              std::move(result_name).release_error(), \
                                              { __FILE__, __LINE__ }); \
        }                               \
    } while(false)

#define RETURN_IMPL(result_name, expr) \
    do {                                \
        auto result_name = (expr); \
        if(result_name.has_failed()) \
        {                                     \
            return detail::make_failed_result(basic_errors::propagated_error{}, \
                                              #expr,                            \
                                              std::move(result_name).release_error(), \
                                              { __FILE__, __LINE__ }); \
        }                              \
        return result_name; \
    } while(false)

#define TRY_ASSIGN(init, expr) TRY_ASSIGN_IMPL(init, TRY_UNIQUE_NAME, expr)

#define TRY(expr) TRY_IMPL(TRY_UNIQUE_NAME, expr)

#define RETURN(expr) RETURN_IMPL(TRY_UNIQUE_NAME, expr)

template<class T>
struct is_result_t : std::bool_constant<false> {};

template<class V, class E, class L>
struct is_result_t<result<V, E, L>> : std::bool_constant<true> {};

#define ERR_2(code, explanation) detail::make_failed_result(code, explanation, { __FILE__, __LINE__ })

#define ERR_3(code, explanation, result_or_data) \
    detail::resolve_failed_result(code, explanation, result_or_data, { __FILE__, __LINE__ });

#define ERR_4(code, explanation, data, result) \
    detail::resolve_failed_result(code, explanation, result, data, { __FILE__, __LINE__ });

#define err( ... ) VA_SELECT( ERR, __VA_ARGS__ )

#define EXPECT_IMPL(result_name, expr, explanation) \
    do {                                            \
        auto&& result_name = (expr); \
        if(!static_cast<bool>(result_name)) \
        {                             \
            return fail_precondition(std::move(result_name), #expr, explanation, { __FILE__, __LINE__ });           \
        }                                               \
    } while(false)                                                \

#define EXPECT(expr, explanation) EXPECT_IMPL(TRY_UNIQUE_NAME, expr, explanation)

#define ENSURE_IMPL(result_name, expr, explanation) \
    do {                                            \
        auto&& result_name = (expr); \
        if(!static_cast<bool>(result_name)) \
        {                             \
            return fail_postcondition(std::move(result_name), #expr, explanation, { __FILE__, __LINE__ });           \
        }                                               \
    } while(false)

#define ENSURE(expr, explanation) ENSURE_IMPL(TRY_UNIQUE_NAME, expr, explanation)

#endif //ERRORHANDLING_MACROS_H
