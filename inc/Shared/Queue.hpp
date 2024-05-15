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
    ~Queue();
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
    std::list<std::shared_ptr<SortableElement>> _elements;
};

template <typename SortableElement> Queue<SortableElement>::Queue() {}

template <typename SortableElement> Queue<SortableElement>::~Queue() { this->_elements.clear(); }

template <typename SortableElement> auto Queue<SortableElement>::begin() -> SortableElementIterator
{
    return _elements.begin();
}

template <typename SortableElement> auto Queue<SortableElement>::end() -> SortableElementIterator
{
    return _elements.end();
}

template <typename SortableElement>
auto Queue<SortableElement>::begin() const -> SortableElementConstIterator
{
    return _elements.begin();
}

template <typename SortableElement>
auto Queue<SortableElement>::end() const -> SortableElementConstIterator
{
    return _elements.end();
}

template <typename SortableElement>
auto Queue<SortableElement>::cbegin() const -> SortableElementConstIterator
{
    return _elements.cbegin();
}

template <typename SortableElement>
auto Queue<SortableElement>::cend() const -> SortableElementConstIterator
{
    return _elements.cend();
}

template <typename SortableElement> void Queue<SortableElement>::append_item(std::shared_ptr<SortableElement> element)
{
    _elements.push_back(element);
}

template <typename SortableElement>
template <typename ElementIDType>
std::shared_ptr<SortableElement> Queue<SortableElement>::find_item(const ElementIDType &id) const
{
    for (const auto &element : _elements)
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
    // std::sort(_elements.begin(), _elements.end(), compareFunc);
    _elements.sort(compareFunc);
}

template <typename SortableElement> bool Queue<SortableElement>::is_empty() const { return _elements.empty(); }

template <typename SortableElement> int Queue<SortableElement>::nb_items() const { return _elements.size(); }
