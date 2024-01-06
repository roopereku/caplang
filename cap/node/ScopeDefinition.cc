#include <cap/node/ScopeDefinition.hh>

namespace cap
{

ScopeDefinition::ScopeDefinition(Type type, Token name)
	: Node(Node::Type::ScopeDefinition), type(type), name(name)
{
}

ScopeDefinition::ScopeDefinition()
	: ScopeDefinition(Type::None, Token::createInvalid())
{
}

std::shared_ptr <Node> ScopeDefinition::getRoot() const
{
	return root;
}

void ScopeDefinition::initializeRoot(std::shared_ptr <Node>&& node)
{
	if(!root)
	{
		root = std::move(node);	
	}
}

const char* ScopeDefinition::getTypeString()
{
	return "Scope";
}

std::shared_ptr <Node> ScopeDefinition::findDefinition(Token name)
{
	auto current = root;

	// Iterate through each node in this scope.
	while(current)
	{
		auto definition = current->isDefinition(name);
		if(definition)
		{
			return definition;
		}

		current = current->getNext();
	}

	// If a parent exists, check if the definition exists there.
	if(!getParent().expired() && getParent().lock()->type == Node::Type::ScopeDefinition)
	{
		return getParent().lock()->as <ScopeDefinition> ()->findDefinition(name);
	}

	return nullptr;
}

std::shared_ptr <Node> ScopeDefinition::isDefinition(Token name)
{
	return this->name == name ? shared_from_this() : nullptr;
}

}
