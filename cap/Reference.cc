#include <cap/Reference.hh>
#include <cap/Node.hh>

#include <cap/node/ScopeDefinition.hh>
#include <cap/node/FunctionDefinition.hh>
#include <cap/node/FunctionSignature.hh>
#include <cap/node/ExpressionRoot.hh>

namespace cap
{

Reference::Reference() : type(Type::None)
{
}

Reference::Reference(Type type, std::shared_ptr <Node> node)
	: referred(node), type(type)
{
}

Reference::Reference(std::shared_ptr <Node> node)
{
	referred = node;

	if(node->type == Node::Type::ScopeDefinition)
	{
		switch(node->as <ScopeDefinition> ()->type)
		{
			case ScopeDefinition::Type::FunctionDefinition:
			{
				type = Type::FunctionDefinition;
				break;
			}

			case ScopeDefinition::Type::TypeDefinition:
			{
				type = Type::TypeDefinition;
				break;
			}

			default: {}
		}
	}

	else if(node->type == Node::Type::Expression &&
			node->as <Expression> ()->type == Expression::Type::Root)
	{
		switch(node->as <ExpressionRoot> ()->type)
		{
			case ExpressionRoot::Type::ParameterDefinition:
			{
				type = Type::Parameter;
				break;
			}

			case ExpressionRoot::Type::VariableDefinition:
			{
				type = Type::Variable;
				break;
			}

			case ExpressionRoot::Type::AliasDefinition:
			{
				type = Type::Alias;
				break;
			}


			default: {}
		}
	}
}

Reference::operator bool()
{
	return type != Type::None;
}

std::shared_ptr <TypeDefinition> Reference::getAssociatedType()
{
	auto node = getReferred();

	if(!node)
	{
		return nullptr;
	}

	switch(type)
	{
		case Type::Parameter:
		case Type::Variable:
		case Type::Alias:
		{
			if(node->as <Expression> ()->getResultType().expired())
			{
				return nullptr;
			}

			return node->as <ExpressionRoot> ()->getResultType().lock();
		}

		case Type::FunctionDefinition:
		{
			return node->as <FunctionDefinition> ()->getSignature();
		}

		case Type::TypeDefinition:
		{
			return node->as <TypeDefinition> ();
		}

		case Type::None: {}
	}

	return nullptr;
}

std::shared_ptr <Node> Reference::getReferred()
{
	return referred.expired() ? nullptr : referred.lock();
}

Token Reference::getReferredName()
{
	auto node = getReferred();
	if(node)
	{
		switch(type)
		{
			case Type::FunctionDefinition:
			case Type::TypeDefinition:
			{
				return node->as <ScopeDefinition> ()->name;
			}

			case Type::Parameter:
			case Type::Variable:
			case Type::Alias:
			{
				return node->as <ExpressionRoot> ()->token;
			}

			default: {}
		}
	}

	return Token::createInvalid();
}

Reference::Type Reference::getType()
{
	return type;
}

const char* Reference::getTypeString()
{
	switch(type)
	{
		case Type::FunctionDefinition: return "Function";
		case Type::TypeDefinition: return "Type";
		case Type::Parameter: return "Parameter";
		case Type::Variable: return "Variable";
		case Type::Alias: return "Alias";
		case Type::None: return "None";
	}

	return "???";
}

}
