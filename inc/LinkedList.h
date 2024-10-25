#pragma once

#include <vector>

/// @brief a general implementation of a linked list
/// @tparam T the type of data stored in the linked list
template <class T>
class LinkedList
{
public:
    LinkedList(T data, LinkedList* next);
    ~LinkedList();
    T getData() const;
    LinkedList* getNext() const;
    
    /// @brief a function for moving the contents of the linked list to a vector
    /// @brief (list is destroyed in the process)
    /// @param list the list to be moved
    /// @return a vector containing the contents of the list
    static std::vector<T> move_to_vec(LinkedList* list);
private:
    T data;
    LinkedList* next;
};


template <class T>
LinkedList<T>::LinkedList(T data, LinkedList<T>* next): data(data), next(next)  {}

template <class T>
LinkedList<T>::~LinkedList()
{
    delete next;
}

template <class T>
inline T LinkedList<T>::getData() const
{
    return data;
}

template <class T>
inline LinkedList<T> *LinkedList<T>::getNext() const
{
    return next;
}

template <class T>
inline std::vector<T> LinkedList<T>::move_to_vec(LinkedList<T> *list)
{
    std::vector<T> result;
    for(LinkedList<T>* i = list; i; i = i->getNext())
        result.push_back(i->getData());
    delete list;
    return result;
}
