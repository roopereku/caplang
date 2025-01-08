#include <cap/Node.hh>
#include <cap/Scope.hh>

#include <cassert>

namespace cap
{

Node::Node(Type type)
	: type(type)
{
}

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

void Node::setToken(Token token)
{
	at = token;
}

Token Node::getToken()
{
	return at;
}

std::shared_ptr <Scope> Node::getParentScope()
{
	// TODO: Avoid recursion?

	if(!parent.expired())
	{
		auto next = parent.lock();

		// If the parent node is a scope, return it.
		// Alternatively go further.
		return next->type == Type::Scope ?
			std::static_pointer_cast <Scope> (next) :
			next->getParentScope();
	}

	// If there is no parent, there is no parent scope.
	return nullptr;
}

}
