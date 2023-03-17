#pragma once

#include <boost/preprocessor.hpp>
#include <functional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

// clang-format off

/*************************** utils ***************************/

namespace reflcpp {
    struct foreach_stop {
        bool value;
    };
}

namespace reflcpp::utils {
    template<typename>
    struct field_type;

    template<typename C, typename T>
    struct field_type<T C::*> {
        using type = T;
    };

    template<typename T>
    struct field_type<T *> {
        using type = T;
    };

    template<typename T>
    using field_type_t = typename field_type<T>::type;

    template<typename T, typename F, std::size_t ...Idx>
    constexpr void tuple_foreach_impl(T &&tuple, F &&func, std::index_sequence<Idx...>) {
        [[maybe_unused]] auto result = (
            [&]() {
                using R = std::invoke_result_t<F, std::tuple_element_t<Idx, T>>;
                if constexpr (std::is_same_v<R, foreach_stop>) {
                    return std::invoke(std::forward<F>(func), std::get<Idx>(std::forward<T>(tuple))).value;
                } else {
                    std::invoke(std::forward<F>(func), std::get<Idx>(std::forward<T>(tuple)));
                    return false;
                }
            }() || ...
        );
    }

    template<typename T, typename F>
    constexpr void tuple_foreach(T &&tuple, F &&func) {
        tuple_foreach_impl(std::forward<T>(tuple), std::forward<F>(func),
                           std::make_index_sequence<std::tuple_size_v<T>>{});
    }
}

/*************************** interface ***************************/

namespace reflcpp {
    enum Access {
        Public    = 0b0001,
        Protected = 0b0010,
        Private   = 0b0100,
        All       = Public | Protected | Private,
    };

    template<typename T>
    constexpr bool is_reflectable_v = false;

    template<typename T, typename Enable = std::enable_if_t<is_reflectable_v<T>, void>>
    class metainfo {
    public:
        constexpr static std::string_view name() noexcept;
        constexpr static auto bases() noexcept;
        constexpr static auto public_fields() noexcept;
        constexpr static auto protected_fields() noexcept;
        constexpr static auto private_fields() noexcept;
    private:
        template<typename> struct baseinfo;
        template<auto> struct fieldinfo;
    };

    template<typename T>
    constexpr std::enable_if_t<is_reflectable_v<T>, std::size_t> 
    bases_size() {
        return std::tuple_size_v<decltype(metainfo<T>::bases())>;
    }

    template<typename T>
    constexpr std::enable_if_t<is_reflectable_v<T>, std::size_t> 
    fields_size(Access access) {
        std::size_t size = 0;
        if (access & Public) {
            size += std::tuple_size_v<decltype(metainfo<T>::public_fields())>;
        }
        if (access & Protected) {
            size += std::tuple_size_v<decltype(metainfo<T>::protected_fields())>;
        }
        if (access & Private) {
            size += std::tuple_size_v<decltype(metainfo<T>::private_fields())>;
        }
        return size;
    }

    template<typename T, typename F>
    constexpr std::enable_if_t<is_reflectable_v<T>, void> 
    bases_foreach(F &&func) {
        utils::tuple_foreach(metainfo<T>::bases(), std::forward<F>(func));
    }

    template<typename T, typename F>
    constexpr std::enable_if_t<is_reflectable_v<T>, void> 
    fields_foreach(F &&func, Access access = Access::All) {
        if (access & Public) {
            utils::tuple_foreach(metainfo<T>::public_fields(), std::forward<F>(func));
        }
        if (access & Protected) {
            utils::tuple_foreach(metainfo<T>::protected_fields(), std::forward<F>(func));
        }
        if (access & Private) {
            utils::tuple_foreach(metainfo<T>::private_fields(), std::forward<F>(func));
        }
    }

    template<typename T, typename F>
    constexpr std::enable_if_t<is_reflectable_v<T>, void> 
    bases_foreach_recursive(F &&func) {
        utils::tuple_foreach(metainfo<T>::bases(), [&](auto base) {
            using BaseType = typename decltype(base)::type;
            std::invoke(std::forward<F>(func), base);
            if constexpr (is_reflectable_v<BaseType>) {
                bases_foreach_recursive<BaseType>(std::forward<F>(func));
            }
        });
    }

    template<typename T, typename F>
    constexpr std::enable_if_t<is_reflectable_v<T>, void> 
    fields_foreach_recursive(F &&func, Access access = Access::All) {
        fields_foreach<T>(std::forward<F>(func), access);
        bases_foreach<T>([&](auto base) {
            using BaseType = typename decltype(base)::type;
            if constexpr (is_reflectable_v<BaseType>) {
                fields_foreach_recursive<BaseType>(std::forward<F>(func), access);
            }
        });
    }
}

#define REFLCPP_METAINFO(...) BOOST_PP_OVERLOAD(REFLCPP_METAINFO_, __VA_ARGS__)(__VA_ARGS__)

/*************************** implement ***************************/

#define REFLCPP_METAINFO_1(Class)                REFLCPP_METAINFO_5(Class, , , , )
#define REFLCPP_METAINFO_2(Class, Bases)         REFLCPP_METAINFO_5(Class, Bases, Public, , )
#define REFLCPP_METAINFO_3(Class, Bases, Public) REFLCPP_METAINFO_5(Class, Bases, Public, , )
#define REFLCPP_METAINFO_5(Class, Bases, Public, Protected, Private)            \
    template<> constexpr inline bool reflcpp::is_reflectable_v<Class> = true;   \
    BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_BASEINFO, Class, Bases)            \
    BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_FIELDINFO, Class, Public)          \
    BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_FIELDINFO, Class, Protected)       \
    BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_FIELDINFO, Class, Private)         \
    template<>                                                                  \
    constexpr std::string_view reflcpp::metainfo<Class>::name() noexcept {      \
        return BOOST_PP_STRINGIZE(Class);                                       \
    }                                                                           \
    template<>                                                                  \
    constexpr auto reflcpp::metainfo<Class>::bases() noexcept {                 \
        return std::make_tuple(                                                 \
            BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_BASE, Class, Bases)        \
        );                                                                      \
    };                                                                          \
    template<>                                                                  \
    constexpr auto reflcpp::metainfo<Class>::public_fields() noexcept {         \
        return std::make_tuple(                                                 \
            BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_FIELD, Class, Public)      \
        );                                                                      \
    }                                                                           \
    template<>                                                                  \
    constexpr auto reflcpp::metainfo<Class>::protected_fields() noexcept {      \
        return std::make_tuple(                                                 \
            BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_FIELD, Class, Protected)   \
        );                                                                      \
    }                                                                           \
    template<>                                                                  \
    constexpr auto reflcpp::metainfo<Class>::private_fields() noexcept{         \
        return std::make_tuple(                                                 \
            BOOST_PP_SEQ_FOR_EACH_I(REFLCPP_METAINFO_FIELD, Class, Private)     \
        );                                                                      \
    }

#define REFLCPP_METAINFO_BASE(r, c, i, b)  BOOST_PP_COMMA_IF(i) baseinfo<b>{}
#define REFLCPP_METAINFO_FIELD(r, c, i, f) BOOST_PP_COMMA_IF(i) fieldinfo<&c::f>{}
#define REFLCPP_METAINFO_BASEINFO(r, c, i, b)                                   \
    template<> template<>                                                       \
    struct reflcpp::metainfo<c>::baseinfo<b> {                                  \
        using type = b;                                                         \
    };
#define REFLCPP_METAINFO_FIELDINFO(r, c, i, f)                                  \
    template<> template<>                                                       \
    struct reflcpp::metainfo<c>::fieldinfo<&c::f> {                             \
        using type = reflcpp::utils::field_type_t<decltype(&c::f)>;             \
        constexpr std::string_view name() const noexcept {                      \
            return BOOST_PP_STRINGIZE(f);                                       \
        }                                                                       \
        constexpr auto ptr() const noexcept {                                   \
            return &c::f;                                                       \
        }                                                                       \
        template<typename C,                                                    \
            std::enable_if_t<std::is_convertible_v<C, c>, int> = 0>             \
        constexpr auto&& get(C &&obj) {                                         \
            return std::forward<C>(obj).f;                                      \
        }                                                                       \
        template<typename C, typename T,                                        \
            std::enable_if_t<std::is_convertible_v<C, c>, int> = 0>             \
        constexpr void set(C &&obj, T &&value) {                                \
            std::forward<C>(obj).f = std::forward<T>(value);                    \
        }                                                                       \
    };
