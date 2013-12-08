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
#include <algorithm>
#include <cassert>

namespace view
{

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
using Ptr_t =
    typename std::add_pointer<
        typename std::decay<T>::type
    >::type;



template <class T>
using ConstRef_t =
    typename std::add_lvalue_reference<
        typename std::add_const<
            typename std::decay<T>::type
        >::type
    >::type;



template <class T>
using ConstPtr_t =
    typename std::add_pointer<
        typename std::add_const<
            typename std::decay<T>::type
        >::type
    >::type;



// View Filter Type and Default Filter

template <class T>
using Filter_t = std::function<bool(ConstRef_t<ElementType_t<T>>)>;

template <class T>
struct DefaultFilter
{
    inline bool operator()(ConstRef_t<ElementType_t<T>>) { return true; }
};



// View Comparator

template <class T>
using Compare_t = std::function<bool(ConstRef_t<ElementType_t<T>>, ConstRef_t<ElementType_t<T>>)>;

template <class T>
using DefaulfCompare = std::less<ConstRef_t<ElementType_t<T>>>;



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
        static R ref(I it) { return *it; }
    };

    template <class I, class R>
    struct IteratorGet<I, R, true>
    {
        static R ref(I it) { return **it; }
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
     * P is the pointer type
     */
    template <class T, template <class ...> class C, class I, class R, class P>
    class IteratorBase
    {
    protected:
        I self; // The underlying iterator
        I end;
        Filter_t<T> filter;

        IteratorBase(I it, I end, Filter_t<T> predicate)
        : self(it), end(end), filter(predicate)
        {
            if (end != self && !filter(getRef()))
                increment();
        }

        void increment()
        {
            // Loop until a valid element is found or the end is reached
            bool loop = self != end;
            while (loop) {
                ++self;
                if (self == end || filter(getRef()))
                    loop = false;
            }
        }

    public:
        IteratorBase& operator++() { increment(); return *this; }
        R getRef() const { assert(self != end); return IteratorGet<I, R, IsPtr<T>::value>::ref(self); }
        P getPtr() const { return &getRef(); }
        R operator*() const { return getRef(); }
        P operator->() const { return getPtr(); }

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
class ConstViewIterator : public priv::IteratorBase<T, C, typename C<T>::const_iterator, ConstRef_t<ElementType_t<T>>, ConstPtr_t<ElementType_t<T>>>
{
public:
    // Import constructor & set up friendship with the view itself
    using priv::IteratorBase<T, C, typename C<T>::const_iterator, ConstRef_t<ElementType_t<T>>, ConstPtr_t<ElementType_t<T>>>::IteratorBase;
    friend View<T, C>;
    friend ConstView<T, C>;
};



// And the non const version

template <class T, template <class...> class C>
class ViewIterator : public priv::IteratorBase<T, C, typename C<T>::iterator, Ref_t<ElementType_t<T>>, Ptr_t<ElementType_t<T>>>
{
public:
    // Import constructor & set up friendship with the view itself
    using priv::IteratorBase<T, C, typename C<T>::iterator, Ref_t<ElementType_t<T>>, Ptr_t<ElementType_t<T>>>::IteratorBase;
    friend View<T, C>;
    friend ConstView<T, C>;

    using Base = priv::IteratorBase<T, C, typename C<T>::iterator, Ref_t<ElementType_t<T>>, Ptr_t<ElementType_t<T>>>;
    operator ConstViewIterator<T, C>() { return { Base::self, Base::end, Base::filter}; }
};



// Views

template <class T, template <class...> class C>
class ConstView
{
protected:
    ConstRef_t<C<T>> underlying_container;
    Filter_t<T> filter;

public:
    using const_iterator = ConstViewIterator<T, C>;

    ConstView(ConstRef_t<C<T>> container, Filter_t<T> predicate)
    : underlying_container(container), filter(predicate)
    { }

    const_iterator begin() const { return { underlying_container.begin(), underlying_container.end(), filter }; }
    const_iterator end()   const { return { underlying_container.end(),   underlying_container.end(), filter }; }

    // Add some algorithms to ease the use of views

    const_iterator min(Compare_t<T> comp = DefaulfCompare<T>())
    {
        return std::min_element(begin(), end(), comp);
    }

    const_iterator max(Compare_t<T> comp = DefaulfCompare<T>())
    {
        return std::max_element(begin(), end(), comp);
    }
};


// A View is a ConstView
template <class T, template <class...> class C>
class View : public ConstView<T, C>
{
    using Base = ConstView<T, C>;
    Ref_t<C<T>> underlying_container;

public:
    using iterator = ViewIterator<T, C>;

    View(Ref_t<C<T>> container, Filter_t<T> predicate)
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
View<T, C> create(C<T>& container, Filter_t<T> filter = DefaultFilter<T>())
{
    return { container, filter };
}



template <class T, template <class...> class C>
ConstView<T, C> create(C<T> const& container, Filter_t<T> filter = DefaultFilter<T>())
{
    return { container, filter };
}



template <class C>
using ViewForContainer_t = decltype(create(std::declval<C&>()));



template <class C>
using ConstViewForContainer_t = decltype(create(std::declval<C const&>()));



#define ViewTypeFromVar(container) view::ViewForContainer_t<decltype(container)>
#define ConstViewTypeFromVar(container) view::ConstViewForContainer_t<decltype(container)>



} // end of namespace view

#endif // CONTAINERVIEW_VIEW_HPP

