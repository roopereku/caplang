#include <cap/Node.hh>

#include <cassert>

namespace cap
{

std::weak_ptr <Node> Node::handleToken(ParserContext&, Token&)
{
	return weak_from_this();
}

std::weak_ptr <Node> Node::getParent() const
{
	return parent;
}

void Node::adopt(std::shared_ptr <Node> node)
{
	assert(node);
	node->parent = shared_from_this();
}

Node::Type Node::getType()
{
	return type;
}

Node::Node(Type type)
	: type(type)
{
}

}
