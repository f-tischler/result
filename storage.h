//
// Created by flori on 03.04.2022.
//

#ifndef ERRORHANDLING_STORAGE_H
#define ERRORHANDLING_STORAGE_H

#include <type_traits>
#include <variant>
#include <optional>
#include <memory>

#if defined(__GNUC__) || defined(__clang__)
#define finline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define finline __forceinline
#else
#define finline
#warning "no forcing of inlining is possible"
#endif

namespace detail
{
    template<class T>
    class sbo_storage
    {
    public:
        sbo_storage() = default;
        explicit sbo_storage(T&& error)
            : m_error(std::move(error))
        {
        }

        template<class...Args>
        sbo_storage(Args&&...args)
            : m_error(T(std::forward<Args&&>(args)...))
        {
        }

        [[nodiscard]] finline bool has_value() const { return m_error.has_value(); }
        [[nodiscard]] finline auto get() const & -> const T& { return m_error.value(); }
        [[nodiscard]] finline auto get() & -> T& { return m_error.value(); }
        [[nodiscard]] finline auto get() && -> T&& { return std::move(m_error).value(); }
        [[nodiscard]] finline T* operator ->() { return &m_error.value(); }

        void reset() { m_error.reset(); }

    private:
        std::optional<T> m_error;
    };

    template<class T>
    class pointer_storage
    {
    public:
        pointer_storage() = default;
        explicit pointer_storage(T&& error)
            : m_data(new T(std::move(error)))
        {
        }

        template<class...Args>
        pointer_storage(Args&&...args)
            : m_data(new T(std::forward<Args&&>(args)...))
        {
        }

        [[nodiscard]] finline bool has_value() const { return m_data != nullptr; }
        [[nodiscard]] finline auto get() const & -> const T& { return *m_data; }
        [[nodiscard]] finline auto get() & -> T& { return *m_data; }
        [[nodiscard]] finline auto get() && -> T&& { return std::move(*m_data); }
        [[nodiscard]] finline T* operator ->() { return m_data.get(); }

        void reset() { m_data.reset(); }

    private:
        std::unique_ptr<T> m_data;
    };

    template<class Value, class Error>
    using result_storage_t = std::conditional_t<sizeof(Error) <= sizeof(Value),
            std::variant<Value, sbo_storage<Error>>,
            std::variant<Value, pointer_storage<Error>>>;

    template<class Value, class Error>
    class result_storage
    {
    public:
        using error_storage_type = std::decay_t<decltype(std::get<1>(std::declval<result_storage_t<Value, Error>>()))>;

        explicit result_storage(Value&& value)
            : m_storage(std::move(value))
        {
        }

        explicit result_storage(Error&& error)
            : m_storage(error_storage_type(std::move(error)))
        {
        }

        [[nodiscard]] finline bool has_value() const { return std::holds_alternative<Value>(m_storage); }
        [[nodiscard]] finline auto get_value() const -> const Value& { return std::get<0>(m_storage); }
        [[nodiscard]] finline auto get_error() const -> const Error& { return std::get<1>(m_storage).get(); }

    private:
        result_storage_t<Value, Error> m_storage;
    };
}

#endif //ERRORHANDLING_STORAGE_H
