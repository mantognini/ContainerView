//
//    ContainerView - Copyright (c) 2013 Marco Antognini <antognini.marco@gmail.com>
//
//    This software is provided 'as-is', without any express or implied warranty. In
//    no event will the authors be held liable for any damages arising from the use
//    of this software.
//
//    Permission is granted to anyone to use this software for any purpose, including
//    commercial applications, and to alter it and redistribute it freely, subject to
//    the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not claim
//       that you wrote the original software. If you use this software in a product,
//       an acknowledgment in the product documentation would be appreciated but is
//       not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.
//



#ifndef CONTAINERVIEW_VIEW_HPP
#define CONTAINERVIEW_VIEW_HPP


#include <iterator>
#include <utility>
#include <type_traits>
#include <memory>
#include <functional>
#include <cassert>



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

// Iterator filter, basic predicate function
// With template specification for shared_ptr

template <class T>
struct ElementType { using type = T; };
template <class T>
struct ElementType<std::shared_ptr<T>> { using type = T; };
template <class T>
using ElementType_t = typename ElementType<T>::type;

template <class T>
using AddConstRef_t = typename std::add_lvalue_reference<typename std::add_const<T>::type>::type;

template <class T>
using IteratorFilter = std::function<bool(AddConstRef_t<ElementType_t<T>>)>;

template <class T>
struct DefaultFilter
{
    inline bool operator()(ElementType_t<T> const&) { return true; }
};

// Iterators for View

template <class T, template <class...> class C>
class ViewIterator : public IteratorFromContainer<C<T>>
{
    using Container = C<T>;
    using type = typename Container::iterator;
    type self; // The underlying iterator
    type end;
    IteratorFilter<T> filter;

    friend View<T, C>;

    ViewIterator(type it, type end, IteratorFilter<T> predicate)
    : self(it), end(end), filter(predicate)
    {
        if (end != self && !filter(get()))
            increment();
    }

    void increment()
    {
        // Loop until a valid element is found or the end is reached
        bool loop = self != end;
        while (loop) {
            ++self;
            if (self == end || filter(get()))
                loop = false;
        }
    }

public:
    using typename IteratorFromContainer<Container>::reference;
    using SELF = ViewIterator<T, C>;

    ViewIterator& operator++() { increment(); return *this; }
    reference get() const  { assert(self != end); return *self; }
    reference operator*() const { return get(); }

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
    type end;
    IteratorFilter<T> filter;

    friend View<T, C>;
    friend ConstView<T, C>;

    ConstViewIterator(type it, type end, IteratorFilter<T> predicate)
    : self(it), end(end), filter(predicate)
    {
        if (end != self && !filter(get()))
            increment();
    }

    void increment()
    {
        // Loop until a valid element is found or the end is reached
        bool loop = self != end;
        while (loop) {
            ++self;
            if (self == end || filter(get()))
                loop = false;
        }
    }

public:
    using const_reference = typename IteratorFromContainer<Container>::value_type const&;
    using SELF = ConstViewIterator<T, C>;

    ConstViewIterator& operator++() { increment(); return *this; }
    const_reference get() const { assert(self != end); return *self; }
    const_reference operator*() const { return get(); }

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
    type end;
    IteratorFilter<T> filter;

    friend View<std::shared_ptr<T>, C>;

    ViewIterator(type it, type end, IteratorFilter<T> predicate)
    : self(it), end(end), filter(predicate)
    {
        if (end != self && !filter(get()))
            increment();
    }

    void increment()
    {
        // Loop until a valid element is found or the end is reached
        bool loop = self != end;
        while (loop) {
            ++self;
            if (self == end || filter(get()))
                loop = false;
        }
    }

public:
    using reference = typename Container::value_type::element_type&;
    using SELF = ViewIterator<std::shared_ptr<T>, C>;

    ViewIterator& operator++() { increment(); return *this; }
    reference get() const { assert(self != end); return **self; }
    reference operator*() const { return get(); }

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
    type end;
    IteratorFilter<T> filter;

    friend View<std::shared_ptr<T>, C>;
    friend ConstView<std::shared_ptr<T>, C>;

    ConstViewIterator(type it, type end, IteratorFilter<T> predicate)
    : self(it), end(end), filter(predicate)
    {
        if (end != self && !filter(get()))
            increment();
    }

    void increment()
    {
        // Loop until a valid element is found or the end is reached
        bool loop = self != end;
        while (loop) {
            ++self;
            if (self == end || filter(get()))
                loop = false;
        }
    }

public:
    using const_reference = typename Container::value_type::element_type const&;
    using SELF = ConstViewIterator<std::shared_ptr<T>, C>;

    ConstViewIterator& operator++() { increment(); return *this; }
    const_reference get() const  { assert(self != end); return **self; }
    const_reference operator*() const { return get(); }

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
    IteratorFilter<T> filter;

public:
    using const_iterator = ConstViewIterator<T, C>;

    ConstView(Container const& container, IteratorFilter<T> predicate)
    : underlying_container(container), filter(predicate)
    { }

    const_iterator begin() const { return { underlying_container.begin(), underlying_container.end(), filter }; }
    const_iterator end() const { return { underlying_container.end(), underlying_container.end(), filter }; }
};


template <class T, template <class...> class C>
class View : public ConstView<T, C>
{
    using Container = C<T>;
    Container& underlying_container;

public:
    using iterator = ViewIterator<T, C>;
    using Base = ConstView<T, C>;

    View(Container& container, IteratorFilter<T> predicate)
    : ConstView<T, C>(container, predicate), underlying_container(container)
    { }

    using Base::begin;
    iterator begin() { return { underlying_container.begin(), underlying_container.end(), Base::filter }; }
    using Base::end;
    iterator end() { return { underlying_container.end(), underlying_container.end(), Base::filter }; }
};



// Views for shared pointer

template <class T, template <class...> class C>
class ConstView<std::shared_ptr<T>, C>
{
protected:
    using Container = C<std::shared_ptr<T>>;
    Container const& underlying_container;
    IteratorFilter<T> filter;

public:
    using const_iterator = ConstViewIterator<std::shared_ptr<T>, C>;

    ConstView(Container const& container, IteratorFilter<T> predicate)
    : underlying_container(container), filter(predicate)
    { }

    const_iterator begin() const { return { underlying_container.begin(), underlying_container.end(), filter }; }
    const_iterator end() const { return { underlying_container.end(), underlying_container.end(), filter }; }
};


template <class T, template <class...> class C>
class View<std::shared_ptr<T>, C> : public ConstView<std::shared_ptr<T>, C>
{
    using Container = C<std::shared_ptr<T>>;
    Container& underlying_container;

public:
    using iterator = ViewIterator<std::shared_ptr<T>, C>;
    using Base = ConstView<std::shared_ptr<T>, C>;

    View(Container& container, IteratorFilter<T> predicate)
    : ConstView<std::shared_ptr<T>, C>(container, predicate), underlying_container(container)
    { }

    using Base::begin;
    iterator begin() { return { underlying_container.begin(), underlying_container.end(), Base::filter }; }
    using Base::end;
    iterator end() { return { underlying_container.end(), underlying_container.end(), Base::filter }; }
};



// Helpers for the user

template <class T, template <class...> class C>
View<T, C> viewOf(C<T>& container, IteratorFilter<T> filter = DefaultFilter<T>())
{
    return { container, filter };
}

template <class T, template <class...> class C>
ConstView<T, C> constViewOf(C<T> const& container, IteratorFilter<T> filter = DefaultFilter<T>())
{
    return { container, filter };
}

template <class C>
using ViewTypeFromCT_t = decltype(viewOf(std::declval<C&>()));

template <class C>
using ConstViewTypeFromCT_t = decltype(constViewOf(std::declval<C const&>()));

#define ViewTypeFromCTVar(container) ViewTypeFromCT_t<decltype(container)>
#define ConstViewTypeFromCTVar(container) ConstViewTypeFromCT_t<decltype(container)>



#endif // CONTAINERVIEW_VIEW_HPP

