#include <cap/Node.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>
#include <cap/DeclarationStorage.hh>

#include <cassert>

namespace cap
{

Node::Node(Type type)
	: Node(type, DeclarationStorage::getInvalid())
{
}

Node::Node(Type type, DeclarationStorage& declStorage)
	: type(type), declStorage(declStorage)
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

std::shared_ptr <Scope> Node::getParentScope() const
{
	auto result = findParentNode([](std::shared_ptr <Node> node) -> bool
	{
		return node->type == Type::Scope;
	});

	return result ? std::static_pointer_cast <Scope> (result) : nullptr;
}

std::shared_ptr <Function> Node::getParentFunction() const
{
	auto result = findParentNode([](std::shared_ptr <Node> node) -> bool
	{
		return node->type == Type::Declaration &&
			std::static_pointer_cast <Declaration> (node)->getType() == Declaration::Type::Function;
	});

	return result ? std::static_pointer_cast <Function> (result) : nullptr;
}

std::shared_ptr <Node> Node::getParentWithDeclarationStorage() const
{
	auto result = findParentNode([](std::shared_ptr <Node> node) -> bool
	{
		return node->declStorage.isValid();
	});

	return result;
}

DeclarationStorage& Node::getParentDeclarationStorage()
{
	return getParentWithDeclarationStorage()->declStorage;
}

DeclarationStorage& Node::getDeclarationStorage()
{
	return declStorage;
}

std::shared_ptr <Node> Node::findParentNode(bool (*filter)(std::shared_ptr <Node>)) const
{
	// TODO: Avoid recursion?

	if(!parent.expired())
	{
		auto next = parent.lock();

		// If the parent node is of desired type, return it.
		// Alternatively recurse further.
		return filter(next) ? next : next->findParentNode(filter);
	}

	// If there is no parent, there is no parent scope.
	return nullptr;
}

}
