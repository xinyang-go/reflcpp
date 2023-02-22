#include <iostream>
#include <sstream>
#include <string>

#include "reflcpp/core.hpp"
#include "reflcpp/serialization.hpp"
#include "reflcpp/yaml.hpp"
#include "reflcpp/runtime.hpp"

struct Creature {
    std::string type;
};
REFLCPP_METAINFO(Creature, , (type))

struct Person : Creature {
    std::string name;
    int age;
    int score;
};
REFLCPP_METAINFO(Person, (Creature), (name)(age)(score))

void example_reflcpp_core() {
    Person bob;
    bob.name = "Bob";
    bob.age = 22;
    bob.score = 100;
    bob.type = "person";
    // iterate fields
    std::cout << "==== iterate fields ====" << std::endl;
    reflcpp::fields_foreach<Person>([&](auto field) {
        std::cout << field.name() << ": " << field.get(bob) << std::endl;
    });
    // iterate fields recursive
    std::cout << "==== iterate fields recursive ====" << std::endl;
    reflcpp::fields_foreach_recursive<Person>([&](auto field) {
        std::cout << field.name() << ": " << field.get(bob) << std::endl;
    });
    // find field
    std::cout << "==== find field 'name' ====" << std::endl;
    reflcpp::fields_foreach<Person>([&](auto field) {
        if constexpr (field.name() == "name") {
            std::cout << field.name() << ": " << field.get(bob) << std::endl;
        }
    });
}

void example_reflcpp_yaml() {
    std::stringstream buffer;
    // save to yaml
    {
        Person bob;
        bob.name = "Bob";
        bob.age = 22;
        bob.score = 100;
        bob.type = "person";
        YAML::Node node;
        node = bob;
        YAML::Emitter emitter(buffer);
        emitter << node;
        std::cout << "==== save to yaml ==== " << std::endl;
        std::cout << buffer.str() << std::endl;
    }
    // load from yaml
    {
        YAML::Node node = YAML::Load(buffer);
        Person bob = node.as<Person>();
        std::cout << "==== load from yaml ==== " << std::endl;
        reflcpp::fields_foreach_recursive<Person>([&](auto field) {
            std::cout << field.name() << ": " << field.get(bob) << std::endl;
        });
    }
}

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
void example_reflcpp_serialization() {
    std::stringstream buffer;
    {
        Person bob;
        bob.name = "Bob";
        bob.age = 22;
        bob.score = 100;
        bob.type = "person";
        boost::archive::binary_oarchive ar(buffer);
        ar << bob;
        std::cout << "==== serialize saved ==== " << std::endl;
        reflcpp::fields_foreach_recursive<Person>([&](auto field) {
            std::cout << field.name() << ": " << field.get(bob) << std::endl;
        });
    }
    {
        Person bob;
        boost::archive::binary_iarchive ar(buffer);
        ar >> bob;
        std::cout << "==== serialize loaded ==== " << std::endl;
        reflcpp::fields_foreach_recursive<Person>([&](auto field) {
            std::cout << field.name() << ": " << field.get(bob) << std::endl;
        });
    }
}

void example_runtime() {
    Person bob;
    bob.name = "Bob";
    bob.age = 22;
    bob.score = 100;
    bob.type = "person";
    // runtime set field
    std::cout << "==== runtime set field 'score' to 90 ====" << std::endl;
    reflcpp::runtime::field_set(bob, "score", 90);
    std::cout << "bob.score: " << bob.score << std::endl;

    /*
     * the following code will throw std::bad_any_cast
     * because field 'score' needs int, while 90.0 is double
     */
    // reflcpp::runtime::field_set(bob, "score", 90.0);
    
    // runtime get field
    // reflcpp::runtime::field_get() will return a std::any
    // which is a copy of original field value
    std::cout << "==== runtime get field 'age' ====" << std::endl;
    auto var = reflcpp::runtime::field_get(bob, "age");
    std::cout << "bob.age: " << std::any_cast<int>(var) << std::endl;
}

int main() {
    example_reflcpp_core();
    example_reflcpp_yaml();
    example_reflcpp_serialization();
    example_runtime();

    return 0;
}