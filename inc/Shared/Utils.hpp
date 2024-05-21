#pragma once

#include <algorithm>
#include <list>
#include <random>
#include <source_location>
#include <string>
#include <type_traits>

template <typename CharT = char> std::basic_string<CharT> source_location_to_string(const std::source_location &loc)
{
    std::string full_function_name = loc.function_name();
    size_t pos = full_function_name.find('(');
    if (pos != std::string::npos)
    {
        return std::basic_string<CharT>(full_function_name.begin(), full_function_name.begin() + pos) + "()";
    } else
    {
        return std::basic_string<CharT>(full_function_name.begin(), full_function_name.end()) + "()";
    }
}

template <typename T> T random_range(T min, T max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<T> distrib(min, max);
    return distrib(gen);
}

template <class Container>
typename Container::mapped_type get_value(const Container &container, const typename Container::key_type &key,
                                          const std::source_location &loc)
{
    typename Container::const_iterator it = container.find(key);
    PPK_ASSERT_ERROR(it != container.cend(), "Invalid key %s", source_location_to_string(loc).c_str());
    return it->second;
}

template <class Container, class T>
void set_value_helper(Container &container, const typename Container::key_type &key, const T &value,
                      const std::source_location &loc)
{
    auto [iterator, emplaced] = container.try_emplace(key, value);
    PPK_ASSERT_ERROR(emplaced, "Value with the given key was found, function %s!",
                     source_location_to_string(loc).c_str());
}

template <class Container, class T = typename Container::mapped_type>
    requires(!std::is_pointer_v<T>)
void set_value(Container &container, const typename Container::key_type &key, const T &value,
               const std::source_location &loc)
{
    set_value_helper(container, key, value, loc);
}

template <class Container, class T = typename Container::mapped_type>
    requires(std::is_pointer_v<T>)
void set_value(Container &container, const typename Container::key_type &key, const T &value,
               const std::source_location &loc)
{
    PPK_ASSERT_ERROR(value != nullptr, "nullptr value is not allowed, function %s",
                     source_location_to_string(loc).c_str());
    set_value_helper(container, key, value, loc);
}

template <typename Container, typename T, typename Field>
Container sort_by_field(const Container &input, Field T::*field_ptr)
{
    Container sorted = input;
    std::ranges::sort(sorted, [&](const T &a, const T &b) { return a.*field_ptr < b.*field_ptr; });
    return sorted;
}