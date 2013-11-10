//
//  main.cpp
//  ContainerView
//
//  Created by Marco Antognini on 10/11/13.
//  Copyright (c) 2013 Marco Antognini. All rights reserved.
//

//#include <iostream>
//#include <typeinfo>
//#include <vector>
//#include <scoped_allocator>
//
//template <template <class...> class C, class T>
//struct Helper
//{
//    using allocator = typename C<T>::allocator_type;
//};
//
//template <class T>
//using Vector = std::vector<T, std::scoped_allocator_adaptor<std::allocator<T>>>;
//
//int main(int argc, const char * argv[])
//{
//    auto const& info = typeid(Helper<Vector, int>::allocator);
//    auto const& info2 = typeid(Helper<std::vector, int>::allocator);
//
//    std::cout << "Name: " << info.name() << std::endl;
//    std::cout << "Name: " << info2.name() << std::endl;
//
//    return 0;
//}



#include "View.hpp"
#include <iostream>
#include <typeinfo>
#include <vector>
#include <memory>

class Element {
    int x;
public:
    Element(int y) : x(y) { }

    void print(int tab) const
    {
        for (int i = 0; i < tab; ++i)
            std::cout << "\t";
        std::cout << "Element::" << x << "\n";
    }

    void foo() { ++x; }
};

using Document = std::vector<Element>;
using DocumentView = ViewTypeFromCT_t<Document>;
using DocumentConstView = ConstViewTypeFromCT_t<Document>;

void loadDocument(Document& doc)
{
    for (auto i : { 2, 3, 5 })
        doc.push_back(i);
}

void updateDocument(DocumentView view)
{
    for (auto& e : view)
        e.foo();
}

void printDocument(DocumentConstView view)
{
    std::cout << "<Document>\n";
    for (auto const& e : view)
        e.print(1);
    std::cout << "</Document>" << std::endl;
}


using AdvancedDocument = std::vector<std::shared_ptr<Element>>;
using AdvancedDocumentView = ViewTypeFromCT_t<AdvancedDocument>;
using AdvancedDocumentConstView = ConstViewTypeFromCT_t<AdvancedDocument>;

void loadDocument(AdvancedDocument& doc)
{
    for (auto i : { 7, 11, 13 })
        doc.emplace_back(std::make_shared<Element>(i));
}

void updateDocument(AdvancedDocumentView view)
{
    for (auto& e : view)
        e.foo();
}

void printDocument(AdvancedDocumentConstView view)
{
    std::cout << "<AdvancedDocument>\n";
    for (auto const& e : view)
        e.print(1);
    std::cout << "</AdvancedDocument>" << std::endl;
}

int main(int, char**) {
    Document xs;
    loadDocument(xs);
    printDocument(viewOf(xs));
    updateDocument(viewOf(xs));
    printDocument(constViewOf(xs));

    AdvancedDocument ys;
    loadDocument(ys);
    printDocument(viewOf(ys));
    updateDocument(viewOf(ys));
    printDocument(constViewOf(ys));

    using V = ViewTypeFromCTVar(xs);
    using CV = ConstViewTypeFromCTVar(xs);

    std::cout << "Name V: " << typeid(V).name() << std::endl;
    std::cout << "Name CV: " << typeid(CV).name() << std::endl;
    std::cout << "Name DocumentView: " << typeid(DocumentView).name() << std::endl;
    std::cout << "Name AdvancedDocumentView: " << typeid(AdvancedDocumentView).name() << std::endl;

    return 0;
}
