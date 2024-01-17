#include <cap/node/ScopeDefinition.hh>

#include <cap/Reference.hh>

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

Reference ScopeDefinition::findDefinition(std::string_view name)
{
	return findDefinition(name, nullptr);
}

Reference ScopeDefinition::findDefinition(std::string_view name, std::shared_ptr <Node> exclusion)
{
	auto current = root;

	// Iterate through each node in this scope.
	while(current)
	{
		// If the current node isn't the exclusion, check if it's the definition.
		if(exclusion != current && current->isDefinition(name))
		{
			return Reference(current);
		}

		current = current->getNext();
	}

	// If a parent exists, check if the definition exists there.
	if(!getParent().expired() && getParent().lock()->type == Node::Type::ScopeDefinition)
	{
		return getParent().lock()->as <ScopeDefinition> ()->findDefinition(name, exclusion);
	}

	return Reference();
}

bool ScopeDefinition::isDefinition(std::string_view name)
{
	return this->name == name;
}

void ScopeDefinition::removeChildNode(std::shared_ptr <Node> node)
{
	std::shared_ptr <Node> previous;
	auto current = root;

	// Iterate all direct child nodes within this scope.
	while(current->getNext())
	{
		// If the node is found, stop iterating.
		if(current == node)
		{
			break;
		}

		previous = current;
		current = current->getNext();
	}

	// If iteration stopped at the desire node, make the next node of the current node
	// the next node of previous node. This leaves the current node out.
	if(current == node)
	{
		// If no previous node is set, iteration stopped at the root.
		// To remove the current root, assign it to the next of root.
		if(!previous)
		{
			root = root->getNext();
			return;
		}

		previous->setNext(current->getNext());
	}
}

bool ScopeDefinition::isValidationComplete()
{
	return validationComplete;
}

void ScopeDefinition::complete()
{
	validationComplete = true;
}

std::shared_ptr <ScopeDefinition> ScopeDefinition::getShared()
{
	static std::shared_ptr <ScopeDefinition> sharedScope = std::make_shared <ScopeDefinition> ();
	return sharedScope;
}

}
