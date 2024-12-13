#include <cap/Node.hh>

#include <cassert>

namespace cap
{

std::weak_ptr <Node> Node::handleToken(ParserContext&, Token&)
{
	return weak_from_this();
}

std::weak_ptr <Node> Node::appendNext(std::shared_ptr <Node>&& node)
{
	if(!next)
	{
		next = std::move(node);
		return next->weak_from_this();
	}

	return next->appendNext(std::move(node));
}

std::weak_ptr <Node> Node::getNext() const
{
	return next;
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

}
