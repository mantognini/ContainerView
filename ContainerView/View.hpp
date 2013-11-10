//
//  View.hpp
//  ContainerView
//
//  Created by Marco Antognini on 10/11/13.
//  Copyright (c) 2013 Marco Antognini. All rights reserved.
//


#ifndef CONTAINERVIEW_VIEW_HPP
#define CONTAINERVIEW_VIEW_HPP



#include <iterator>
#include <utility>
#include <type_traits>
#include <memory>



// Predeclarations

template <class T, template <class...> class C>
class View;

template <class T, template <class...> class C>
class ConstView;



// Helpers

template <class C>
using IteratorCategory = typename C::iterator::iterator_category;

template <class C>
using IteratorType = typename C::iterator::value_type;

template <class C>
using IteratorDistance = typename C::iterator::difference_type;

template <class C>
using IteratorPointer = typename C::iterator::pointer;

template <class C>
using IteratorReference = typename C::iterator::reference;

template <class C>
using IteratorFromContainer = std::iterator<IteratorCategory<C>, IteratorType<C>, IteratorDistance<C>, IteratorPointer<C>, IteratorReference<C>>;



// Iterators for View

template <class T, template <class...> class C>
class ViewIterator : public IteratorFromContainer<C<T>>
{
    using Container = C<T>;
    using type = typename Container::iterator;
    type self; // The underlying iterator

    friend View<T, C>;

    ViewIterator(type it) : self(it) { }

public:
    using typename IteratorFromContainer<Container>::reference;
    using SELF = ViewIterator<T, C>;

    ViewIterator& operator++() { ++self; return *this; }
    reference operator*() { return *self; }

    // Free function !=
    friend
    bool operator!=(SELF const& a, SELF const& b)
    {
        return a.self != b.self;
    }
};



// Basically the same as ViewIterator but returns const values

template <class T, template <class...> class C>
class ConstViewIterator : public IteratorFromContainer<C<T>>
{
    using Container = C<T>;
    using type = typename Container::const_iterator;
    type self; // The underlying iterator

    friend View<T, C>;
    friend ConstView<T, C>;

    ConstViewIterator(type it) : self(it) { }

public:
    using const_reference = typename IteratorFromContainer<Container>::value_type const&;
    using SELF = ConstViewIterator<T, C>;

    ConstViewIterator& operator++() { ++self; return *this; }
    const_reference operator*() const { return *self; }

    // Free function !=
    friend
    bool operator!=(SELF const& a, SELF const& b)
    {
        return a.self != b.self;
    }
};



// Iterator on shared_ptr

template <class T, template <class...> class C>
class ViewIterator<std::shared_ptr<T>, C> : public IteratorFromContainer<C<std::shared_ptr<T>>>
{
    using Container = C<std::shared_ptr<T>>;
    using type = typename Container::iterator;
    type self; // The underlying iterator

    friend View<std::shared_ptr<T>, C>;

    ViewIterator(type it) : self(it) { }

public:
    using reference = typename Container::value_type::element_type&;
    using SELF = ViewIterator<std::shared_ptr<T>, C>;

    ViewIterator& operator++() { ++self; return *this; }
    reference operator*() { return **self; }

    // Free function !=
    friend
    bool operator!=(SELF const& a, SELF const& b)
    {
        return a.self != b.self;
    }
};

template <class T, template <class...> class C>
class ConstViewIterator<std::shared_ptr<T>, C> : public IteratorFromContainer<C<std::shared_ptr<T>>>
{
    using Container = C<std::shared_ptr<T>>;
    using type = typename Container::const_iterator;
    type self; // The underlying iterator

    friend View<std::shared_ptr<T>, C>;
    friend ConstView<std::shared_ptr<T>, C>;

    ConstViewIterator(type it) : self(it) { }

public:
    using const_reference = typename Container::value_type::element_type const&;
    using SELF = ConstViewIterator<std::shared_ptr<T>, C>;

    ConstViewIterator& operator++() { ++self; return *this; }
    const_reference operator*() const { return **self; }

    // Free function !=
    friend
    bool operator!=(SELF const& a, SELF const& b)
    {
        return a.self != b.self;
    }
};



// Views

template <class T, template <class...> class C>
class ConstView
{
protected:
    using Container = C<T>;
    Container const& underlying_container;

public:
    using const_iterator = ConstViewIterator<T, C>;

    ConstView(Container const& container) : underlying_container(container) { }

    const_iterator begin() const { return { underlying_container.begin() }; }
    const_iterator end() const { return { underlying_container.end() }; }
};


template <class T, template <class...> class C>
class View : public ConstView<T, C>
{
    using Container = C<T>;
    Container& underlying_container;

public:
    using iterator = ViewIterator<T, C>;
    using Base = ConstView<T, C>;

    View(Container& container) : ConstView<T, C>(container), underlying_container(container) { }

    using Base::begin;
    iterator begin() { return { underlying_container.begin() }; }
    using Base::end;
    iterator end() { return { underlying_container.end() }; }
};



// Views for shared pointer

template <class T, template <class...> class C>
class ConstView<std::shared_ptr<T>, C>
{
protected:
    using Container = C<std::shared_ptr<T>>;
    Container const& underlying_container;

public:
    using const_iterator = ConstViewIterator<std::shared_ptr<T>, C>;

    ConstView(Container const& container) : underlying_container(container) { }

    const_iterator begin() const { return { underlying_container.begin() }; }
    const_iterator end() const { return { underlying_container.end() }; }
};


template <class T, template <class...> class C>
class View<std::shared_ptr<T>, C> : public ConstView<std::shared_ptr<T>, C>
{
    using Container = C<std::shared_ptr<T>>;
    Container& underlying_container;

public:
    using iterator = ViewIterator<std::shared_ptr<T>, C>;
    using Base = ConstView<std::shared_ptr<T>, C>;

    View(Container& container) : ConstView<std::shared_ptr<T>, C>(container), underlying_container(container) { }

    using Base::begin;
    iterator begin() { return { underlying_container.begin() }; }
    using Base::end;
    iterator end() { return { underlying_container.end() }; }
};



// Helpers for the user

template <class T, template <class...> class C>
View<T, C> viewOf(C<T>& container) {
    return { container };
}

template <class T, template <class...> class C>
ConstView<T, C> constViewOf(C<T> const& container) {
    return { container };
}

template <class C>
using ViewTypeFromCT_t = decltype(viewOf(std::declval<C&>()));

template <class C>
using ConstViewTypeFromCT_t = decltype(constViewOf(std::declval<C const&>()));

#define ViewTypeFromCTVar(container) ViewTypeFromCT_t<decltype(container)>
#define ConstViewTypeFromCTVar(container) ConstViewTypeFromCT_t<decltype(container)>



#endif // CONTAINERVIEW_VIEW_HPP

