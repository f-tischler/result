//
// Created by flori on 23.04.2022.
//
#ifndef ERRORHANDLING_TYPES_H
#define ERRORHANDLING_TYPES_H

#include <type_traits>

namespace detail {

    struct default_final_action {
        template<class T>
        constexpr void operator()(const T &) const {};
    } g_default_final_action;

    template<class T, class R>
    constexpr inline bool is_final_action_v =
            std::is_invocable_v<T, const R &> &&
            std::is_class_v<T> &&
            std::is_default_constructible_v<T>;
}

class error;

template<class Value = void, class Error = error, class FinalAction = detail::default_final_action>
class result;

#endif //ERRORHANDLING_TYPES_H
