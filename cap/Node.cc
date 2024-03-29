#include <cap/Node.hh>
#include <cap/Reference.hh>

#include <cassert>

namespace cap
{

Node::Node() : Node(Type::Empty)
{
}

Node::Node(Type type) : Node(type, Token::createInvalid())
{
}

Node::Node(Type type, Token token) : type(type), token(token)
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

void Node::removeChildNode(std::shared_ptr <Node> node)
{
	assert(false);
}

const char* Node::getTypeString()
{
	return "???";
}

bool Node::isDefinition(std::string_view)
{
	return false;
}

Reference Node::findNextDefinition(std::string_view name)
{
	auto current = next;
	while(current)
	{
		if(current->isDefinition(name))
		{
			return current;
		}

		current = current->getNext();
	}

	return Reference();
}

}
