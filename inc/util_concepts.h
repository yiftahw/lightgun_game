#include <concepts>
#include <map>
#include <unordered_map>
#include <string>

template <typename C>
concept AssociativeContainer =
    requires(C c, typename C::key_type k) {
        typename C::key_type;
        typename C::mapped_type;
        typename C::value_type;

        { c.find(k) } -> std::same_as<typename C::iterator>;
        { c.begin() } -> std::same_as<typename C::iterator>;
        { c.end() }   -> std::same_as<typename C::iterator>;
    };
