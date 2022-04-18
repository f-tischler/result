//
// Created by flori on 03.04.2022.
//

#ifndef ERRORHANDLING_ERROR_H
#define ERRORHANDLING_ERROR_H

#include <sstream>
#include <cxxabi.h>
#include <fmt/core.h>

struct error_category
{
public:
    constexpr error_category(const int32_t id,
                             const std::string_view name)
        : m_id(id)
        , m_name(name)
    {
    }

    [[nodiscard]] constexpr auto get_id() const -> int32_t { return m_id; }
    [[nodiscard]] constexpr auto get_name() const -> std::string_view { return m_name; }

    [[nodiscard]] constexpr bool operator==(const error_category& rhs) const { return m_id == rhs.m_id; }

private:
    int32_t m_id;
    std::string_view m_name;
};

template<uint32_t Id>
struct error_category_base
    : error_category
{
    constexpr error_category_base(std::string_view name)
        : error_category(Id, name) {}
};

template<uint32_t LocalId, uint32_t CategoryId>
constexpr uint64_t to_global_id(error_category_base<CategoryId>)
{
    return LocalId + (static_cast<uint64_t>(CategoryId) << 32);
}

class error_code
{
public:
    constexpr error_code(const error_category category,
                         const uint64_t global_id,
                         const std::string_view name,
                         const std::string_view description)
        : m_category(category)
        , m_global_id(global_id)
        , m_name(name)
        , m_description(description)
    {
    }

    [[nodiscard]] constexpr auto get_category() const -> const error_category& { return m_category; }
    [[nodiscard]] constexpr auto get_id() const -> uint64_t { return m_global_id; }
    [[nodiscard]] constexpr auto get_name() const -> std::string_view { return m_name; }
    [[nodiscard]] constexpr auto get_description() const -> std::string_view { return m_description; }

    finline constexpr operator uint64_t() const { return m_global_id; } // NOLINT(google-explicit-constructor)

private:
    error_category m_category;
    uint64_t m_global_id;
    std::string_view m_name;
    std::string_view m_description;
};

template<uint32_t LocalId, class Category>
struct error_code_base : error_code
{
    explicit constexpr error_code_base(const std::string_view name,
                                       const std::string_view description)
        : error_code(Category{}, id, name, description)
    {
    }

    static constexpr uint64_t id = to_global_id<LocalId>(Category{});
    static constexpr uint32_t category_id = Category::id;
};

//std::string to_string(const std::type_info& type)
//{
//    size_t size = 256;
//    char* buf = static_cast<char*>(malloc(size));
//    int status;
//
//    buf = abi::__cxa_demangle(type.name(), buf, &size, &status);
//
//    if(status != 0)
//        return "<invalid name>";
//
//    std::string output(buf);
//    free(buf);
//    auto pos = output.rfind("::");
//    if(pos == std::string::npos)
//        return output;
//
//    return output.substr(pos + 2);
//}

struct source_location
{
    const char* file;
    int line;
};

class error
{
public:
    template<class ErrorCode>
    error(ErrorCode&& code, source_location src_loc)
        : m_code(std::forward<ErrorCode>(code))
        , m_src_loc(src_loc)
        , m_inner_error(nullptr)
    {
    }

    template<class ErrorCode>
    error(ErrorCode&& code, std::string explanation, source_location src)
        : m_code(std::forward<ErrorCode>(code))
        , m_src_loc(src)
        , m_explanation(std::move(explanation))
        , m_inner_error(nullptr)
    {
    }

    template<class ErrorCode>
    error(ErrorCode&& code, std::unique_ptr<error>&& inner_error, source_location src_loc)
        : m_code(std::forward<ErrorCode>(code))
        , m_src_loc(src_loc)
        , m_inner_error(std::move(inner_error))
    {
    }

    template<class ErrorCode>
    error(ErrorCode&& code, std::string explanation, std::unique_ptr<error>&& inner_error, source_location src)
        : m_code(std::forward<ErrorCode>(code))
        , m_src_loc(src)
        , m_explanation(std::move(explanation))
        , m_inner_error(std::move(inner_error))
    {
    }

    [[nodiscard]] finline auto get_code() const -> const error_code& { return m_code; }
    [[nodiscard]] finline auto get_explanation() const -> std::string_view { return m_explanation; }
    [[nodiscard]] finline auto get_origin() const -> const source_location& { return m_src_loc; }
    [[nodiscard]] finline auto get_inner_error() const -> const error* { return m_inner_error.get(); }

    finline operator uint64_t() const { return m_code.get_id(); } // NOLINT(google-explicit-constructor)

private:
    error_code m_code;
    source_location m_src_loc;
    std::string m_explanation;
    std::unique_ptr<error> m_inner_error;
};

#endif //ERRORHANDLING_ERROR_H
