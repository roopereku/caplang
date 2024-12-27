#include <cap/Node.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/Expression.hh>

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

bool Node::Traverser::traverseNode(std::shared_ptr <Node> node)
{
	switch(node->getType())
	{
		case Node::Type::Scope: return traverseScope(std::static_pointer_cast <Scope> (node));
		case Node::Type::Expression: return traverseExpression(std::static_pointer_cast <Expression> (node));
		case Node::Type::Custom: onCustomNode(node);
	}

	return true;
}

bool Node::Traverser::traverseScope(std::shared_ptr <Scope> node)
{
	bool traverseNested = false;

	switch(node->getType())
	{
		case Scope::Type::Standalone:
		{
			traverseNested = onScope(node);
			break;
		}

		case Scope::Type::Function:
		{
			traverseNested = onFunction(std::static_pointer_cast <Function> (node));
			// TODO: If traverseNested, traverse to the return value and signature?
			break;
		}

		case Scope::Type::ClassType:
		{
			traverseNested = onClassType(std::static_pointer_cast <ClassType> (node));
			// TODO: If traverseNested, traverse to the base classes?
			break;
		}

		case Scope::Type::Custom:
		{
			traverseNested = onCustomScope(node);
			break;
		}
	}

	if(traverseNested)
	{
		for(auto nested : node->getNested())
		{
			// TODO: Check return value?
			traverseNode(nested);
		}
	}

	return true;
}

bool Node::Traverser::traverseExpression(std::shared_ptr <Expression> node)
{
	onExpression(node);
	return true;
}

void Node::Traverser::onCustomNode(std::shared_ptr <Node>) {}
bool Node::Traverser::onScope(std::shared_ptr <Scope>) { return true; }
bool Node::Traverser::onFunction(std::shared_ptr <Function>) { return true; }
bool Node::Traverser::onClassType(std::shared_ptr <ClassType>) { return true; }
bool Node::Traverser::onCustomScope(std::shared_ptr <Scope>) { return true; }
void Node::Traverser::onExpression(std::shared_ptr <Expression>) {}

}
