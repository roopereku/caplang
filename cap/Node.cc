#include <cap/Node.hh>

namespace cap
{

Node::Node() : type(Type::Empty)
{
}

Node::Node(Type type) : type(type)
{
}

std::shared_ptr <Node> Node::findLast()
{
	std::shared_ptr <Node> current = shared_from_this();

	while(current->next)
	{
		current = current->next;
	}

	return current;
}

void Node::adopt(std::shared_ptr <Node> node)
{
	node->parent = shared_from_this();
}

std::weak_ptr <Node> Node::getParent() const
{
	return parent;
}

std::shared_ptr <Node> Node::getNext() const
{
	return next;
}

void Node::setNext(std::shared_ptr <Node>&& node)
{
	next = std::move(node);
}

}
