#pragma once

#include "core.hpp"

#include <any>
#include <unordered_map>

// clang-format off

namespace reflcpp::runtime {

template<typename T, typename F>
bool field_set(T &obj, const std::string &name, F &&var) {
    using SetterFunc = void (*)(T&, std::any);
    static const std::unordered_map<std::string, SetterFunc> setter = []() {
        std::unordered_map<std::string, SetterFunc> setter;
        fields_foreach_recursive<T>([&](auto field) {
            using FieldInfo = decltype(field);
            setter.emplace(field.name().data(), +[](T &obj, std::any value) {
                FieldInfo{}.set(obj, std::any_cast<typename FieldInfo::type>(std::move(value)));
            });
        });
        return setter;
    }();

    auto it = setter.find(name);
    if (it == setter.end()) return false;
    it->second(obj, std::forward<F>(var));
    return true;
}

template<typename T>
std::any field_get(T &&obj, const std::string &name) {
    using GetterFunc = std::any(*)(T);
    static const std::unordered_map<std::string, GetterFunc> getter = []() {
        std::unordered_map<std::string, GetterFunc> getter;
        fields_foreach_recursive<std::decay_t<T>>([&](auto field) {
            using FieldInfo = decltype(field);
            getter.emplace(field.name().data(), +[](T &&obj) -> std::any {
                return FieldInfo{}.get(std::forward<T>(obj));
            });
        });
        return getter;
    }();

    auto it = getter.find(name);
    if (it == getter.end()) return {};
    return it->second(std::forward<T>(obj));
}

} // namespace reflcpp::runtime
