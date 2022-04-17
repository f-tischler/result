//
// Created by flori on 15.04.2022.
//
#ifndef ERRORHANDLING_FORMATTING_H
#define ERRORHANDLING_FORMATTING_H

#include "result.h"
#include <fmt/format.h>

template<>
struct fmt::formatter<error>
    : formatter<string_view>
{
    template <typename FormatContext>
    auto format(const error& e, FormatContext& ctx)
    {
        return format_to(ctx.out(),
                         "Error '{}' occurred at {}:{}\n"
                         "    Description:     {}\n"
                         "{}"
                         "    Category:        {}\n",
                         e.get_code().get_name(),
                         e.get_origin().file,
                         e.get_origin().line,
                         e.get_code().get_description(),
                         e.get_explanation().empty()
                           ? ""
                           : fmt::format("    Additional Info: {}\n", e.get_explanation()),
                         e.get_code().get_category().get_name());
    }
};

template <class V, class L>
struct fmt::formatter<result<V, error, L>>
    : formatter<error>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(const result<V, error, L>& r, FormatContext& ctx)
    {
        if(r.is_ok())
        {
            return format_to(ctx.out(), "ok");
        }

        return formatter<error>::format(r.get_error(), ctx);
    }
};

#endif //ERRORHANDLING_FORMATTING_H
