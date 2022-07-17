#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS

#include <iostream>
#include <cassert>

#include <doctest/doctest.h>
#include <regex>

#define ASSERTIONS_TERMINATE

#include "result.h"
#include "formatting.h"
#include "macros.h"

namespace errors
{
    DEFINE_ERROR_CATEGORY(3, general_error_category);
    DEFINE_ERROR_CODE(1, general_error_category, unknown_error, "Undefined error");
    DEFINE_ERROR_CODE(2, general_error_category, invalid_pointer_error, "Null pointer make_failure");
    DEFINE_ERROR_CODE(3, general_error_category, argument_out_of_range_error, "Argument out of range");
    DEFINE_ERROR_CODE(4, general_error_category, not_implemented_error, "Function not implemented");
}

struct LogErrorOnDestruction
{
    void operator()(const auto &r) const noexcept
    {
        if (r.has_failed())
        {
            fmt::print("{}\n", r);
        }
    }
};

struct FailTestOnError
{
    void operator()(const auto &r) const noexcept
    {
        if (r.has_failed())
        {
            FAIL_CHECK(fmt::format("unhandled error:\n{}\n", r));
        }
    }
};

static_assert(std::is_class_v<LogErrorOnDestruction>);
static_assert(std::is_default_constructible_v<LogErrorOnDestruction>);

template<class V = void>
using mresult = result<V, error, LogErrorOnDestruction>;

mresult<> ok_result() { return ok(); }
mresult<> failed_result() { return err(errors::unknown_error{}, "failure"); }
mresult<int> ok_int_result() { return ok(1); }
mresult<int> failed_int_result() { return err(errors::unknown_error{}, "failed_int_result"); }

TEST_CASE( "Basic properties of result<>" )
{
    REQUIRE( ok_result().is_ok() );
    REQUIRE( !ok_result().has_failed() );
    REQUIRE( failed_result().has_failed() );
    REQUIRE( !failed_result().is_ok() );
}

TEST_CASE( "Basic properties of result<int>" )
{
    REQUIRE( ok_int_result().is_ok() );
    REQUIRE( ok_int_result().get_value() == 1 );
    REQUIRE( !ok_int_result().has_failed()  );
    REQUIRE( failed_int_result().has_failed() );
    REQUIRE( !failed_int_result().is_ok() );
}

TEST_CASE( "Error handling macros" )
{
    REQUIRE( []() -> mresult<>
             {
                 TRY(ok_result());
                 return ok();
             }().is_ok() );

    REQUIRE( []() -> mresult<>
             {
                 TRY(failed_result());
                 return ok();
             }().has_failed() );

    const auto test_ok_int_result = []() -> mresult<int>
    {
        TRY_ASSIGN(const auto i, ok_int_result());
        return ok(i);
    };

    REQUIRE( test_ok_int_result().is_ok() );
    REQUIRE( test_ok_int_result().get_value() == 1 );

    REQUIRE( []() -> mresult<>
             {
                 TRY(failed_int_result());
                 return ok();
             }().has_failed() );
}

TEST_CASE( "Error message format" )
{
    mresult<> r = err(errors::unknown_error{}, "UNIT TEST");
    std::string s = fmt::format("{}", r);
    fmt::print("{}\n", s);

    //    Error 'unknown_error' occurred at /mnt/d/Dev/Projects/ErrorHandling/main.cpp:71
    //    Description:     Undefined make_failure
    //    Additional Info: UNIT TEST
    //    Category:        general_error_category

    std::cmatch m;

    REQUIRE( std::regex_search(s.data(), m,
                               std::regex("\\s*'unknown_error' at [a-zA-Z0-9/\\.]+:[0-9]+\n")) );

    REQUIRE( std::regex_search(s.data(), m,
                               std::regex("\\s*Description:     Undefined error\n")) );

    REQUIRE( std::regex_search(s.data(), m,
                               std::regex("\\s*Additional Info: UNIT TEST\n")) );

    REQUIRE( std::regex_search(s.data(), m,
                               std::regex("\\s*Category:        general_error_category")) );

    r.dismiss();
}

TEST_CASE( "Propagate make_failure message format" )
{
    mresult<> r = []() -> mresult<>
    {
        TRY([]() -> mresult<>
        {
            TRY([]() -> mresult<>
            {
                TRY([]() -> mresult<>
                {
                    return err(errors::unknown_error{}, "UNIT TEST");
                }());

                return ok();
            }());

            return ok();
        }());

        return ok();
    }();

    std::string s = fmt::format("{}", r);
    fmt::print("{}\n", s);

    r.dismiss();
}

TEST_CASE( "Nested error message format" )
{
    auto try_f = [](auto f)
    {
        return [=]() -> mresult<>
        {
            TRY(std::invoke(f)); return ok();
        };
    };

    auto wrap = [](auto f)
    {
        return [=]() -> mresult<>
        {
            TRY(std::invoke(f).handle_error([](auto &&) -> mresult<>
            {
                return err(errors::unknown_error{}, "wrapper make_failure");
            }));

            return ok();
        };
    };

    mresult<> r = std::invoke(
        try_f(
            try_f(
                try_f(
                    wrap(
                        try_f([]() -> mresult<> { return err(errors::not_implemented_error{}, ""); }))))));

    fmt::print("{}\n", r);
    r.dismiss();
}

TEST_CASE( "Assertions" )
{
    const auto ok_result_precond = []() -> mresult<> { EXPECT(ok_result(), ""); return ok(); };
    const auto failed_result_precond = []() -> mresult<> { EXPECT(failed_result(), ""); ; return ok();};
    const auto ok_precond = []() -> mresult<> { EXPECT(true, ""); return ok(); };
    const auto failed_precond = []() -> mresult<> { EXPECT(false, ""); ; return ok();};

    REQUIRE_NOTHROW( ok_result_precond().dismiss() );
    REQUIRE_THROWS( failed_result_precond().dismiss() );
    REQUIRE_NOTHROW( ok_precond().dismiss() );
    REQUIRE_THROWS( failed_precond().dismiss() );

    const auto ok_result_postcond = []() -> mresult<> { ENSURE(ok_result(), "");  return ok();};
    const auto failed_result_postcond = []() -> mresult<> { ENSURE(failed_result(), ""); return ok(); };
    const auto ok_postcond = []() -> mresult<> { ENSURE(true, ""); return ok(); };
    const auto failed_postcond = []() -> mresult<> { ENSURE(false, ""); ; return ok();};

    REQUIRE_NOTHROW( ok_result_postcond().dismiss() );
    REQUIRE_THROWS( failed_result_postcond().dismiss() );
    REQUIRE_NOTHROW( ok_postcond().dismiss() );
    REQUIRE_THROWS( failed_postcond().dismiss() );
}

TEST_CASE( "Valid error data" )
{
    error e{errors::unknown_error{}, {__FILE__, __LINE__} };

    e.set_data(std::string("test"));

    REQUIRE("test" == e.get_data<std::string>());
}

TEST_CASE( "Invalid error data" )
{
    error e{errors::unknown_error{}, {__FILE__, __LINE__} };

    e.set_data(1);

    REQUIRE_THROWS(e.get_data<std::string>().resize(1));
}

TEST_CASE( "Creating failed result data" )
{
    mresult<> r = err(errors::unknown_error{}, "this is test failure", 1);
    REQUIRE(r.get_error().get_data<int>() == 1);
}

TEST_CASE( "Propagate failed result data" )
{
    const auto fail_with_data = []() -> mresult<> { return err(errors::unknown_error{}, "this is test failure", 1); };

    auto r = [=]() -> mresult<> { TRY(fail_with_data()); return ok(); }();

    REQUIRE_THROWS((void)r.get_error().get_data<int>());
    REQUIRE(r.get_error().get_inner_error() != nullptr);
    REQUIRE(r.get_error().get_inner_error()->get_data<int>() == 1);
}

TEST_CASE( "Handle error using 'handle_error'")
{
    using namespace errors;

    mresult<> r = err(invalid_pointer_error{}, "this is test failure");
    REQUIRE(r.handle_error(
        [](const auto& e) -> result<>
        {
            switch (e)
            {
                case invalid_pointer_error{}:   return ok();
                default:                        return err(unknown_error{}, "unable to handle make_failure");
            }
        }).is_ok());
}

TEST_CASE( "Handle error using 'handle_error' - map error")
{
    using namespace errors;

    mresult<> r = err(not_implemented_error{}, "this is test failure that can not be handled");
    REQUIRE(r.handle_error(
            [](const auto& e) -> result<>
            {
                switch (e)
                {
                    case invalid_pointer_error{}:   return ok();
                    default:                        return err(unknown_error{}, "unable to handle make_failure");
                }
            }).has_failed());
}

//
//result<> foo()
//{
//    return ok();
//}
//
//result<> foo2()
//{
//    return err(errors::not_implemented_error{}, "");
//}
//
//result<int> foo3()
//{
//    return ok(1);
//}
//
//result<> foo4()
//{
//    const auto r = foo3();
//    if(r.has_failed())
//        return detail::make_failure(r.get_error());
//
//    return ok();
//}
//
//result<> foo5()
//{
//    TRY(foo2());
//    return ok();
//}
//
//result<> foo6()
//{
//    RETURN(foo2());
//}
//
//result<int> foo7()
//{
//    TRY_ASSIGN(const auto i, foo3());
//    return ok(i);
//}
//
//result<int> foo8()
//{
//    RETURN(foo3());
//}
//
//result<> foo9()
//{
//    auto r = foo2();
//    if (r.has_failed())
//    {
//        switch (r.get_error().get_code())
//        {
//            case errors::not_implemented_error{}:   return ok();
//            default:                                return err(errors::unknown_error{}, "unable to recover");
//        }
//    }
//
//    return r;
//}
//

//
//result<std::string> foo11()
//{
//    RETURN(foo3().map([](auto value) { return std::to_string(value); }));
//}
//
//result<> foo12()
//{
//    EXPECT(foo2(), "foo2 must not fail");
//
//    return ok();
//}
//
//result<> foo13()
//{
//    foo2().ignore();
//    return ok();
//}
//
//result<> foo14()
//{
//    EXPECT(foo2().is_ok(), "foo2 must not fail");
//    return ok();
//}
//
//result<> foo15()
//{
//    ENSURE(foo2(), "foo2 must not fail");
//    return ok();
//}

//int main()
//{
//    assert(foo().is_ok());
//    assert(foo2().has_failed());
//    std::cout << foo2().get_error().to_string() << std::endl;
//    assert(foo3().is_ok());
//    assert(foo3().get_value() == 1);
//    assert(foo4().is_ok());
//    assert(foo5().has_failed());
//    std::cout << foo5().get_error().to_string() << std::endl;
//    assert(foo6().has_failed());
//    assert(foo7().get_value() == 1);
//    assert(foo8().get_value() == 1);
//    assert(foo9().is_ok());
//    assert(foo10().has_failed());
//    assert(foo11().get_value() == "1");
//
//    try
//    {
//        assert(foo12().has_failed());
//        assert(false);
//    }
//    catch(const std::exception& ex)
//    {
//        std::cout << ex.what() << std::endl;
//    }
//
//    assert(foo13().is_ok());
//
//    try
//    {
//        assert(foo14().has_failed());
//        assert(false);
//    }
//    catch(const std::exception& ex)
//    {
//        std::cout << ex.what() << std::endl;
//    }
//
//    try
//    {
//        assert(foo15().has_failed());
//        assert(false);
//    }
//    catch(const std::exception& ex)
//    {
//        std::cout << ex.what() << std::endl;
//    }
//}
