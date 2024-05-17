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
    void append_item(std::shared_ptr<SortableElement> element);
    template <typename ElementIDType> std::shared_ptr<SortableElement> find_item(const ElementIDType &id) const;
    template <typename CompareFunc> void sort_queue(CompareFunc compareFunc);
    bool is_empty() const;
    int nb_items() const;

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

template <typename SortableElement> void Queue<SortableElement>::append_item(std::shared_ptr<SortableElement> element)
{
    elements.push_back(element);
}

template <typename SortableElement>
template <typename ElementIDType>
std::shared_ptr<SortableElement> Queue<SortableElement>::find_item(const ElementIDType &id) const
{
    for (const auto &element : elements)
    {
        if (element->get_id() == id)
        {
            return element;
        }
    }
    return nullptr;
}

template <typename SortableElement>
template <typename CompareFunc>
void Queue<SortableElement>::sort_queue(CompareFunc compareFunc)
{
    elements.sort(compareFunc);
}

template <typename SortableElement> bool Queue<SortableElement>::is_empty() const { return elements.empty(); }

template <typename SortableElement> int Queue<SortableElement>::nb_items() const { return elements.size(); }
