#pragma once

#include <yaml-cpp/yaml.h>

#include "core.hpp"

// clang-format off

namespace reflcpp::yaml {
    template<typename T>
    std::enable_if_t<is_reflectable_v<T>, YAML::Node> encode(const T &obj) {
        YAML::Node node;
        fields_foreach_recursive<T>([&](auto field) {
            node[field.name().data()] = field.get(obj);
        });
        return node;
    }

    template<typename T>
    std::enable_if_t<is_reflectable_v<T>, bool> decode(const YAML::Node& node, T& obj) {
        fields_foreach_recursive<T>([&](auto field) {
            field.set(obj, node[field.name().data()].template as<typename decltype(field)::type>());
        });
        return true;
    }
}

#define REFLCPP_YAML(Class, ...)                                                \
    template<> struct YAML::convert<Class> {                                    \
        static Node encode(const Class &obj) {                                  \
            return reflcpp::yaml::encode(obj);                                  \
        }                                                                       \
        static bool decode(const Node& node, Class& obj) {                      \
            return reflcpp::yaml::decode(node, obj);                            \
        }                                                                       \
    }; 
