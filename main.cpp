#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS

#include <iostream>
#include <cassert>

#include <doctest/doctest.h>
#include <regex>

#include "result.h"
#include "formatting.h"
#include "macros.h"

namespace errors
{
    DEFINE_ERROR_CATEGORY(2, general_error_category);
    DEFINE_ERROR_CODE(1, general_error_category, unknown_error, "Undefined error");
    DEFINE_ERROR_CODE(2, general_error_category, invalid_pointer_error, "Null pointer error");
    DEFINE_ERROR_CODE(3, general_error_category, argument_out_of_range_error, "Argument out of range");
    DEFINE_ERROR_CODE(4, general_error_category, not_implemented_error, "Function not implemented");
}

result<> ok_result() { return ok(); }
result<> failed_result() { return err(errors::unknown_error{}, ""); }
result<int> ok_int_result() { return ok(1); }
result<int> failed_int_result() { return err(errors::unknown_error{}, ""); }

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
    REQUIRE( []() -> result<>
             {
                 TRY(ok_result());
                 return ok();
             }().is_ok() );

    REQUIRE( []() -> result<>
             {
                 TRY(failed_result());
                 return ok();
             }().has_failed() );

    const auto test_ok_int_result = []() -> result<int>
    {
        TRY_ASSIGN(const auto i, ok_int_result());
        return ok(i);
    };

    REQUIRE( test_ok_int_result().is_ok() );
    REQUIRE( test_ok_int_result().get_value() == 1 );

    REQUIRE( []() -> result<>
             {
                 TRY(failed_int_result());
                 return ok();
             }().has_failed() );
}

TEST_CASE( "Error message format" )
{
    const result<> r = err(errors::unknown_error{}, "UNIT TEST");
    fmt::print("{}", r.get_error().to_string());

    //    Error 'unknown_error' occurred at /mnt/d/Dev/Projects/ErrorHandling/main.cpp:71
    //    Description:     Undefined error
    //    Additional Info: UNIT TEST
    //    Category:        general_error_category

    std::cmatch m;

    REQUIRE( std::regex_search(r.get_error().to_string().data(), m,
                               std::regex("\\s*Error 'unknown_error' occurred at [a-zA-Z0-9/\\.]+:[0-9]+\n")) );

    REQUIRE( std::regex_search(r.get_error().to_string().data(), m,
                               std::regex("\\s*Description:     Undefined error\n")) );

    REQUIRE( std::regex_search(r.get_error().to_string().data(), m,
                               std::regex("\\s*Additional Info: UNIT TEST\n")) );

    REQUIRE( std::regex_search(r.get_error().to_string().data(), m,
                               std::regex("\\s*Category:        general_error_category")) );
}


TEST_CASE( "Assertions" )
{
    const auto ok_result_precond = []() -> result<> { EXPECT(ok_result(), ""); return ok(); };
    const auto failed_result_precond = []() -> result<> { EXPECT(failed_result(), ""); ; return ok();};
    const auto ok_precond = []() -> result<> { EXPECT(true, ""); return ok(); };
    const auto failed_precond = []() -> result<> { EXPECT(false, ""); ; return ok();};

    REQUIRE_NOTHROW( ok_result_precond().ignore() );
    REQUIRE_THROWS( failed_result_precond().ignore() );
    REQUIRE_NOTHROW( ok_precond().ignore() );
    REQUIRE_THROWS( failed_precond().ignore() );

    const auto ok_result_postcond = []() -> result<> { ENSURE(ok_result(), "");  return ok();};
    const auto failed_result_postcond = []() -> result<> { ENSURE(failed_result(), ""); return ok(); };
    const auto ok_postcond = []() -> result<> { ENSURE(true, ""); return ok(); };
    const auto failed_postcond = []() -> result<> { ENSURE(false, ""); ; return ok();};

    REQUIRE_NOTHROW( ok_result_postcond().ignore() );
    REQUIRE_THROWS( failed_result_postcond().ignore() );
    REQUIRE_NOTHROW( ok_postcond().ignore() );
    REQUIRE_THROWS( failed_postcond().ignore() );
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
//        return detail::error(r.get_error());
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
//result<> foo10()
//{
//    RETURN(foo2().handle_error(
//        [](auto e) -> result<>
//        {
//            switch (e)
//            {
//                case errors::invalid_pointer_error{}:   return ok();
//                default:                                return err(errors::unknown_error{}, "unable to handle error");
//            }
//        }));
//}
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
