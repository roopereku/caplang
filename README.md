# Caplang

Caplang is a library that compiles code written in Cap into an easily accessible AST.

## Usage

### Getting started

Add the following to your `CMakeLists.txt` after adding a target.

```cmake
add_subdirectory(caplang)
target_link_libraries(MyTarget cap)
```

Programs can be compiled as follows:
```cpp
#include <cap/SourceFile.hh>

// Class for handling events sent by cap.
class EventHandler : public cap::EventEmitter
{
public:
    // Handle the event here.
    void emit(cap::Event&& event)
    {
        if(event == Cap::Event::Type::Message)
        {
            // Do something with a message.
            auto msg = event.as <cap::Message> ();
        }
    }
};

int main()
{
    cap::SourceFile sourceFile("SomeFile.cap");

    EventHandler events;
    if(!sourceFile.prepare(events))
    {
        return 1;
    }

    // Do something with the global scope.
    traverseNodes(sourceFile.getGlobal());
}
```

### Traversing the AST

Every node in the AST is of type `cap::Node`. More specific information can be found out by checking the `type` field in a node.

```cpp
void traverseNodes(std::shared_ptr <cap::Node> node)
{
    switch(node->type)
    {
        // The node is a scope.
        case cap::Node::Type::ScopeDefinition:
        {
            switch(node->as <ScopeDefinition> ()->type)
            {
                // The scope is a type.
                case cap::Node::TypeDefinition:
                {
                    break;
                }
            }

            break;
        }

        // The node contains an expression.
        case cap::Node::Type::Expression:
        {
            break;
        }
    }

    // Recursively handle the next node if there is any.
    if(node->getNext())
    {
        traverseNodes(node->getNext());
    }
}