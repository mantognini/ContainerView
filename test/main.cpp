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



#include "View.hpp"
#include <iostream>
#include <typeinfo>
#include <vector>
#include <memory>

class Element
{
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

    int getX() const { return x; }
};

using Document = std::vector<Element>;
using DocumentView = ViewTypeFromCT_t<Document>;
using DocumentConstView = ConstViewTypeFromCT_t<Document>;

// Loading need the vector itself since it needs to add elements
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

// Loading need the vector itself since it needs to add elements
void loadDocument(AdvancedDocument& doc)
{
    for (auto i : { 7, 11, 18 })
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

int main(int, char**)
{
    Document xs;
    loadDocument(xs);
    printDocument(viewOf(xs));  // Ok, auto conversion to const view
    updateDocument(viewOf(xs));
    printDocument(constViewOf(xs));

    printDocument(constViewOf(xs, [](Element const& e) { /*std::cout << "Testing..." << e.getX(); */ return e.getX() % 2 == 0; }));

    AdvancedDocument ys;
    loadDocument(ys);
    printDocument(viewOf(ys));  // Ok, auto conversion to const view
    updateDocument(viewOf(ys));
    printDocument(constViewOf(ys));

    printDocument(constViewOf(ys, [](Element const& e) { /*std::cout << "Testing..." << e.getX(); */ return e.getX() % 2 == 0; }));

    using V = ViewTypeFromCTVar(xs);
    using CV = ConstViewTypeFromCTVar(xs);

    std::cout << "Name V: " << typeid(V).name() << std::endl;
    std::cout << "Name CV: " << typeid(CV).name() << std::endl;
    std::cout << "Name DocumentView: " << typeid(DocumentView).name() << std::endl;
    std::cout << "Name AdvancedDocumentView: " << typeid(AdvancedDocumentView).name() << std::endl;

    return 0;
}
