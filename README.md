# reflcpp: 一个基于宏的cpp静态反射工具

一个基于宏的静态反射工具，非侵入式。同时尽可能减少额外代码编写量。

## 功能列表

* 基类（递归）静态遍历、查找。
* 成员（递归）静态遍历、查找。
* 成员动态读取和赋值。
* 支持非public成员（需要声明friend）。
* yaml-cpp集成。
* boost.serialization集成。

> **递归：**在进行基类、成员遍历的过程中，会向基类进行递归遍历。

更多集成TODO：

* fmt格式化输出
* 

Limitations：

* 不支持引用类型的成员变量
* 不支持数组类型的成员变量（请使用std::array作为替代）

## 示例

```cpp
struct Person {
    std::string name;
    int age;
};
/* 使用宏REFLCPP_METAINFO注册类型信息
 * REFLCPP_METAINFO(<类名>, <基类>, <public成员>, <protected成员>, <private成员>)
 */
REFLCPP_METAINFO(Person, , (name)(age))


int main() {
    // reflcpp::fields_size<T>函数获取类型T的成员数量
    std::cout << reflcpp::fields_size<Person>() << std::endl;
    // 使用reflcpp::Public指定只获取public成员的数量
    std::cout << reflcpp::fields_size<Person>(reflcpp::Public) << std::endl;
  
    // reflcpp::fields_foreach<T>函数遍历类型T的所有成员
    Person tony{.name = "Tony", .age = 23};
    reflcpp::fields_foreach<Person>([&](auto field) {
        /* field.name()获取成员变量名
	 * field.get(obj)获取obj变量的对应成员
         *
         * 当field.name()=="age"时, field.get(obj)等价于obj.age
         */ 
        std::cout << field.name() << ": " << field.get(tony) << std::endl;
        // typename decltype(field)::type为成员变量的类型
	using field_type = typename decltype(field)::type;
    });

    // 使用reflcpp::fields_foreach<T>和if constexpr实现成员静态查找
    reflcpp::fields_foreach<Person>([&](auto field) {
        if constexpr(field.name() == "name") {
            std::cout << field.name() << ": " << field.get(tony) << std::endl;
        }
    });
}
```

更多示例见[example/example.cpp](example/example.cpp)

## 依赖库

* boost.preprocessor
* yaml-cpp ( optional )
* boost.serialization ( optional )
* fmt ( optional )
