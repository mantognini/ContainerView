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



// Type Helpers

template <class T>
struct ElementType { using type = T; };

template <class T>
struct ElementType<std::shared_ptr<T>> { using type = T; };

template <class T>
using ElementType_t = typename ElementType<T>::type;



template <class T>
using Ref_t =
    typename std::add_lvalue_reference<
        typename std::decay<T>::type
    >::type;



template <class T>
using ConstRef_t =
    typename std::add_lvalue_reference<
        typename std::add_const<
            typename std::decay<T>::type
        >::type
    >::type;



// View Filter Type and Default Filter

template <class T>
using IteratorFilter = std::function<bool(ConstRef_t<ElementType_t<T>>)>;

template <class T>
struct DefaultFilter
{
    inline bool operator()(ConstRef_t<ElementType_t<T>>) { return true; }
};



// Iterator Helper

namespace priv
{
    /*
     * Get the reference to the value of the iterator.
     * I is the iterator type
     * R is the reference type
     * IsPtr is a boolean indicating if an extra dereference is required
     */
    template <class I, class R, bool IsPtr>
    struct IteratorGet
    {
        static R get(I it) { return *it; }
    };

    template <class I, class R>
    struct IteratorGet<I, R, true>
    {
        static R get(I it) { return **it; }
    };



    template <class T>
    struct IsPtr : public std::false_type { };

    template <class T>
    struct IsPtr<std::shared_ptr<T>> : public std::true_type { };



    /*
     * Base structure for view iterator.
     * T is the element type of the container
     * C is the container template type
     * I is the iterator type (e.g. const_iterator)
     * R is the reference type (e.g. const ref)
     */
    template <class T, template <class ...> class C, class I, class R>
    class IteratorBase
    {
    protected:
        I self; // The underlying iterator
        I end;
        IteratorFilter<T> filter;

        IteratorBase(I it, I end, IteratorFilter<T> predicate)
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
        IteratorBase& operator++() { increment(); return *this; }
        R get() const { assert(self != end); return IteratorGet<I, R, IsPtr<T>::value>::get(self); }
        R operator*() const { return get(); }

        // Free function !=
        friend
        bool operator!=(IteratorBase const& a, IteratorBase const& b)
        {
            return a.self != b.self;
        }
    };
}



// Iterators for View

template <class T, template <class...> class C>
class ViewIterator : public priv::IteratorBase<T, C, typename C<T>::iterator, Ref_t<ElementType_t<T>>>
{
public:
    // Import constructor & set up friendship with the view itself
    using priv::IteratorBase<T, C, typename C<T>::iterator, Ref_t<ElementType_t<T>>>::IteratorBase;
    friend View<T, C>;
    friend ConstView<T, C>;
};



// Basically the same as ViewIterator but returns const values

template <class T, template <class...> class C>
class ConstViewIterator : public priv::IteratorBase<T, C, typename C<T>::const_iterator, ConstRef_t<ElementType_t<T>>>
{
public:
    // Import constructor & set up friendship with the view itself
    using priv::IteratorBase<T, C, typename C<T>::const_iterator, ConstRef_t<ElementType_t<T>>>::IteratorBase;
    friend View<T, C>;
    friend ConstView<T, C>;
};



// Views

template <class T, template <class...> class C>
class ConstView
{
protected:
    ConstRef_t<C<T>> underlying_container;
    IteratorFilter<T> filter;

public:
    using const_iterator = ConstViewIterator<T, C>;

    ConstView(ConstRef_t<C<T>> container, IteratorFilter<T> predicate)
    : underlying_container(container), filter(predicate)
    { }

    const_iterator begin() const { return { underlying_container.begin(), underlying_container.end(), filter }; }
    const_iterator end()   const { return { underlying_container.end(),   underlying_container.end(), filter }; }
};


// A View is a ConstView
template <class T, template <class...> class C>
class View : public ConstView<T, C>
{
    using Base = ConstView<T, C>;
    Ref_t<C<T>> underlying_container;

public:
    using iterator = ViewIterator<T, C>;

    View(Ref_t<C<T>> container, IteratorFilter<T> predicate)
    : Base(container, predicate), underlying_container(container)
    { }

    iterator begin() { return { underlying_container.begin(), underlying_container.end(), Base::filter }; }
    iterator end()   { return { underlying_container.end(),   underlying_container.end(), Base::filter }; }

    // Import const versions of begin() and end()
    using Base::begin;
    using Base::end;
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

