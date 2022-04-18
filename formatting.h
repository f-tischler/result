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
        std::string indent;
        return format_error(e, indent, ctx);
    }


    template <typename FormatContext>
    auto format_error(const error& e, std::string& indent, FormatContext& ctx, bool cause = false)
    {
        std::vector<const error*> propagations;
        auto inner = &e;
        for(; inner->get_inner_error() && *inner == basic_errors::propagated_error{};
            inner = inner->get_inner_error())
        {
            propagations.push_back(inner);
        }

        const auto format = [&ctx, &e = *inner, &indent, cause]()
        {
            return format_to(ctx.out(),
                            "{}{}'{}' at {}:{}\n"
                            "{}    Description:     {}\n"
                            "{}"
                            "{}    Category:        {}\n",
                            indent.data(),
                            cause ? "| caused by " : "",
                            e.get_code().get_name(),
                            e.get_origin().file,
                            e.get_origin().line,
                            indent.data(),
                            e.get_code().get_description(),
                            e.get_explanation().empty()
                                ? ""
                                : fmt::format("{}    Additional Info: {}\n",
                                              indent.data(),
                                              e.get_explanation()),
                            indent.data(),
                            e.get_code().get_category().get_name());
        };

        auto it = format();

        if(!propagations.empty())
        {
            it = format_to(ctx.out(), "{}    + Error Trace: \n", indent.data());

            std::for_each(cbegin(propagations), cend(propagations), [&](auto p)
            {
                it = format_to(ctx.out(),
                               "{}    | at {}:{}\n",
                               indent.data(),
                               p->get_origin().file,
                               p->get_origin().line);
            });
        }


        if(!inner->get_inner_error())
        {
            return it;
        }

        indent += "    ";
        return format_error(*inner->get_inner_error(), indent, ctx, true);

//        if(e == basic_errors::propagated_error{} &&
//           *e.get_inner_error() == basic_errors::propagated_error{})
//        {
//            indent += "    ";
//            format_error(*e.get_inner_error(), indent, ctx);
//        }
//        else
//        {
//            indent = "";
//            format_error(*e.get_inner_error(), indent, ctx);
//        }
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
