ContainerView
=============

Container view for C++ container (e.g. std::vector) with read-write or read-only access, and smart with container of shared_ptr.


Why this project?
-----------------

While I was developing another project, I was concerned about const correctness: when sharing data (e.g. `std::vector<std::shared_ptr<T>>`) to another function you can make sure the vector is not modified with a const ref. But the elements of the vector can still be modified (e.g. `*vec[0] = 42;`).

Here is an example of what **I don't like**:

```cpp
#include <iostream>
#include <vector>
#include <memory>

using XS = std::vector<std::shared_ptr<int>>;

void foo(XS const& xs) {
    for (auto& x : xs) {
        *x = 42;
    }
}

void print(XS const& xs) {
    std::cout << "<XS>\n";
    for (auto const& x : xs) {
        std::cout << "\t" << *x << "\n";
    }
    std::cout << "</XS>\n";
}

int main(int, char**) {
    XS xs;
    for (auto i : { 1, 2, 3}) {
        xs.push_back(std::make_shared<int>(i));
    }

    std::cout << "before:\n";
    print(xs);
    foo(xs);
    std::cout << "after:\n";
    print(xs);

    return 0;
}
```

[This example is live on ideone.com](http://ideone.com/r1zwWC). You can also read the [SO question](http://stackoverflow.com/q/19616586/520217) I started.


Use Case
--------

I also wanted to hide the pointers. So that a view of `shared_ptr<T>` will be like a view of `T`.

```cpp
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

int main(int, char**)
{
    Document xs;
    loadDocument(xs);
    printDocument(viewOf(xs));  // Ok, auto conversion to const view
    updateDocument(viewOf(xs));
    printDocument(constViewOf(xs));

    AdvancedDocument ys;
    loadDocument(ys);
    printDocument(viewOf(ys));  // Ok, auto conversion to const view
    updateDocument(viewOf(ys));
    printDocument(constViewOf(ys));

    return 0;
}
```

As you can see, views of `Document` or `AdvancedDocument` are used the same way (i.e. no extra pointer dereferencing).
