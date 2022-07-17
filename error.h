//
// Created by flori on 03.04.2022.
//

#ifndef ERRORHANDLING_ERROR_H
#define ERRORHANDLING_ERROR_H

#include <sstream>
#include <cxxabi.h>
#include <any>

//#include <backward.hpp>

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
    explicit constexpr error_category_base(std::string_view name)
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

    [[nodiscard]] finline constexpr auto get_category() const -> const error_category& { return m_category; }
    [[nodiscard]] finline constexpr auto get_id() const -> uint64_t { return m_global_id; }
    [[nodiscard]] finline constexpr auto get_name() const -> std::string_view { return m_name; }
    [[nodiscard]] finline constexpr auto get_description() const -> std::string_view { return m_description; }
    [[nodiscard]] finline constexpr operator uint64_t() const { return m_global_id; } // NOLINT(google-explicit-constructor)

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

struct source_location
{
    const char* file;
    int line;
};

class error
{
public:
    template<class ErrorCode>
    error(ErrorCode&& code, source_location origin)
        : error(std::forward<ErrorCode&&>(code), {}, nullptr, origin)
    {
//        m_bt.load_here();
    }

    template<class ErrorCode>
    error(ErrorCode&& code, std::string explanation, source_location origin)
        : error(std::forward<ErrorCode&&>(code), std::move(explanation), nullptr, origin)
    {
//        m_bt.load_here();
    }

    template<class ErrorCode>
    error(ErrorCode&& code, std::unique_ptr<error>&& inner_error, source_location origin)
        : error(std::forward<ErrorCode&&>(code), {}, std::move(inner_error), origin)
    {
    }

    template<class ErrorCode>
    error(ErrorCode&& code,
          std::string explanation,
          std::unique_ptr<error>&& inner_error,
          source_location origin)
        : m_code(std::forward<ErrorCode&&>(code))
        , m_origin(origin)
        , m_explanation(std::move(explanation))
        , m_inner_error(std::move(inner_error))
    {
    }

    template<class ErrorCode, class Data>
    error(ErrorCode&& code,
          std::string explanation,
          std::unique_ptr<error>&& inner_error,
          source_location origin,
          Data&& data)
        : m_code(std::forward<ErrorCode&&>(code))
        , m_origin(origin)
        , m_explanation(std::move(explanation))
        , m_inner_error(std::move(inner_error))
        , m_data(std::forward<Data>(data))
    {
    }

    error(error&& e, std::unique_ptr<error>&& inner_error)
        : error(std::move(e))
    {
        m_inner_error = std::move(inner_error);
    }

    [[nodiscard]] finline auto get_code() const -> const error_code& { return m_code; }
    [[nodiscard]] finline auto get_explanation() const -> std::string_view { return m_explanation; }
    [[nodiscard]] finline auto get_origin() const -> const source_location& { return m_origin; }
    [[nodiscard]] finline auto get_inner_error() const -> const error* { return m_inner_error.get(); }
    [[nodiscard]] finline operator uint64_t() const { return m_code.get_id(); } // NOLINT(google-explicit-constructor)

    template<typename T>
    [[nodiscard]] finline auto get_data() -> T& { return any_cast<T&>(m_data); }

    template<typename T>
    [[nodiscard]] finline auto get_data() const -> const T& { return any_cast<const T&>(m_data); }

    [[nodiscard]] finline bool has_data() const { return m_data.has_value(); }
    [[nodiscard]] finline auto get_data_type() const -> std::string_view { return m_data.type().name(); }

    template<typename T>
    finline error& set_data(T&& data) { m_data = std::forward<T&&>(data); return *this; }

    finline error& set_inner_error(std::unique_ptr<error> inner) { m_inner_error = std::move(inner); return *this; }

private:
    error_code m_code;
    source_location m_origin;
    std::string m_explanation;
    std::unique_ptr<error> m_inner_error;
    std::any m_data;
//    backward::StackTrace m_bt;
};

template<auto> struct _size{};

_size<sizeof(error)> s;
_size<sizeof(std::any)> s1;
_size<sizeof(std::string)> s2;
_size<sizeof(std::unique_ptr<error>)> s3;
_size<sizeof(error_code)> s4;

#endif //ERRORHANDLING_ERROR_H
