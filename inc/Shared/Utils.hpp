#pragma once

#include <algorithm>
#include <list>
#include <random>
#include <string>
#include <type_traits>

static std::random_device rd;

template <typename T, typename Compare = std::less<T>>
void generate_unique_numbers(std::list<T> &list, size_t count, T min, T max, Compare comp = std::less<T>())
{
    std::uniform_int_distribution<T> dist(min, max);
    std::mt19937 gen(rd());
    while (list.size() < count)
    {
        T num = dist(gen);
        if (std::find(list.begin(), list.end(), num) == list.end())
        {
            list.push_back(num);
        }
    }
    list.sort(comp);
}

template <typename T> T random_range(T min, T max)
{
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<T> distrib(min, max);
    return distrib(gen);
}

template <class Container>
typename Container::mapped_type get_value(const Container &container, const typename Container::key_type &key,
                                          std::string called_from)
{
    typename Container::const_iterator it = container.find(key);
    PPK_ASSERT_ERROR(it != container.cend(), "Invalid key %s", called_from.c_str());
    return it->second;
}

template <class Container>
void set_value_helper(Container &container, const typename Container::key_type &key,
                      const typename Container::mapped_type &value, std::string called_from)
{
    auto p = container.insert({key, value});
    PPK_ASSERT_ERROR(p.second != false, "Value with the given key was found, function %s!", called_from.c_str());
}

template <class Container, class T = typename Container::mapped_type>
void set_value(Container &container, const typename Container::key_type &key,
               const typename std::enable_if<std::is_pointer<T>::value, T>::type &value, std::string called_from)
{
    PPK_ASSERT_ERROR(value != nullptr, "nullptr value is not allowed, function %s", called_from.c_str());
    set_value_helper(container, key, value, called_from);
}

template <class Container, class T = typename Container::mapped_type>
void set_value(Container &container, const typename Container::key_type &key,
               const typename std::enable_if<!std::is_pointer<T>::value, T>::type &value, std::string called_from)
{
    set_value_helper(container, key, value, called_from.c_str());
}

template <typename T, typename Field> struct SortByField
{
    Field T::*field_ptr;
    bool operator()(const T &a, const T &b) const { return a.*field_ptr < b.*field_ptr; }
};

template <typename Container, typename T, typename Field>
Container sort_by_field(const Container &input, Field T::*field_ptr)
{
    Container sorted = input;
    std::sort(sorted.begin(), sorted.end(), [&](const T &a, const T &b) { return a.*field_ptr < b.*field_ptr; });
    return sorted;
}