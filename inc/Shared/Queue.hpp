#pragma once

#include "External/pempek_assert.hpp"
#include <algorithm>
#include <list>
#include <memory>
#include <vector>

template <typename SortableElement> class Queue
{
  public:
    Queue();
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    using SortableElementIterator = std::list<std::shared_ptr<SortableElement>>::iterator;
    using SortableElementConstIterator = std::list<std::shared_ptr<SortableElement>>::const_iterator;
    SortableElementIterator begin();
    SortableElementIterator end();
    SortableElementConstIterator begin() const;
    SortableElementConstIterator end() const;
    SortableElementConstIterator cbegin() const;
    SortableElementConstIterator cend() const;
    using SearchResult = std::pair<SortableElementConstIterator, bool>;
    void append_element(const std::shared_ptr<SortableElement> &element);
    template <typename ElementIDType> SearchResult element_exists(const ElementIDType &id) const;
    template <typename ElementIDType> std::shared_ptr<const SortableElement> get_element(const ElementIDType &id) const;
    template <typename CompareFunc> void sort_queue(CompareFunc compareFunc);
    bool is_empty() const;
    size_t nb_items() const;

  private:
    std::list<std::shared_ptr<SortableElement>> elements;
};

template <typename SortableElement> Queue<SortableElement>::Queue() = default;

template <typename SortableElement> auto Queue<SortableElement>::begin() -> SortableElementIterator
{
    return elements.begin();
}

template <typename SortableElement> auto Queue<SortableElement>::end() -> SortableElementIterator
{
    return elements.end();
}

template <typename SortableElement> auto Queue<SortableElement>::begin() const -> SortableElementConstIterator
{
    return elements.begin();
}

template <typename SortableElement> auto Queue<SortableElement>::end() const -> SortableElementConstIterator
{
    return elements.end();
}

template <typename SortableElement> auto Queue<SortableElement>::cbegin() const -> SortableElementConstIterator
{
    return elements.cbegin();
}

template <typename SortableElement> auto Queue<SortableElement>::cend() const -> SortableElementConstIterator
{
    return elements.cend();
}

template <typename SortableElement>
void Queue<SortableElement>::append_element(const std::shared_ptr<SortableElement> &element)
{
    const auto &[iterator, exists] = this->element_exists(element->id);
    PPK_ASSERT_ERROR(!exists, "element already exists");
    elements.emplace_back(element);
}

template <typename SortableElement>
template <typename ElementIDType>
Queue<SortableElement>::SearchResult Queue<SortableElement>::element_exists(const ElementIDType &id) const
{
    const auto &it = std::ranges::find_if(elements, [&id](const auto &element) { return element->id == id; });
    return (it != elements.end()) ? std::make_pair(it, true) : std::make_pair(elements.cend(), false);
}

template <typename SortableElement>
template <typename ElementIDType>
std::shared_ptr<const SortableElement> Queue<SortableElement>::get_element(const ElementIDType &id) const
{
    const auto &[it, exists] = this->element_exists(id);
    return exists ? *it : nullptr;
}

template <typename SortableElement>
template <typename CompareFunc>
void Queue<SortableElement>::sort_queue(CompareFunc compareFunc)
{
    elements.sort(compareFunc);
}

template <typename SortableElement> bool Queue<SortableElement>::is_empty() const { return elements.empty(); }

template <typename SortableElement> size_t Queue<SortableElement>::nb_items() const { return elements.size(); }
