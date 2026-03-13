#include <cap/Node.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>
#include <cap/DeclarationStorage.hh>
#include <cap/ParserContext.hh>
#include <cap/Validator.hh>
#include <cap/Client.hh>
#include <cap/Value.hh>

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

std::weak_ptr <Node> Node::invokedNodeExited(Node::ParserContext&, Token&)
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

std::pair <size_t, size_t> Node::getAttributeRange() const
{
	return attributeRange;
}

void Node::setAttributeRange(std::pair <size_t, size_t> range)
{
	attributeRange = range;
}

bool Node::validateAttributes(Validator& validator)
{
	auto& ctx = validator.getParserContext();
	auto attributes = ctx.client.getAttributes(shared_from_this());

	for(auto& attribute : attributes)
	{
		// Before validation someone has to adopt the attribute.
		assert(!attribute->getParent().expired());

		if(!attribute->validate(validator))
		{
			return false;
		}

		// TODO: Make sure that an attribute whose declaration isn't a function isn't called.

		// Let different node implementations handle builtin attribute types in their own way.
		if(auto builtinAttr = Builtin::getAttributeType(attribute->getReferred()->getName()))
		{
			if(!handleBuiltinAttribute(validator, *builtinAttr, attribute))
			{
				return false;
			}
		}
	}

	return true;
}

bool Node::handleBuiltinAttribute(Validator&, Builtin::AttributeType, std::shared_ptr <Attribute>)
{
	assert(false);
	return false;
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
