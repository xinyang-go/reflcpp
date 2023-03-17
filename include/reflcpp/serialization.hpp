#pragma once

#include <boost/serialization/base_object.hpp>

#include "core.hpp"

// clang-format off

namespace reflcpp::serialization {
template <typename Archive, typename T>
typename std::enable_if_t<reflcpp::is_reflectable_v<T>, void>
serialize(Archive &ar, T &obj, const unsigned int version) {
    reflcpp::bases_foreach<T>([&](auto base) {
        using BaseType = typename decltype(base)::type;
        ar &boost::serialization::base_object<BaseType>(obj);
    });
    reflcpp::fields_foreach<T>([&](auto field) { ar &field.get(obj); });
}
} // namespace reflcpp::serialization

#define REFLCPP_SERIALIZATION(Class, ...)                                       \
    namespace boost::serialization {                                            \
        template<typename Archive>                                              \
        void serialize(                                                         \
                Archive &ar, Class &obj, const unsigned int version) {          \
            reflcpp::serialization::serialize(ar, obj, version);                \
        }                                                                       \
    }
