#include <cap/Variable.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Validator.hh>
#include <cap/Value.hh>
#include <cap/Identifier.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/Client.hh>
#include <cap/Scope.hh>

#include <cassert>

namespace cap
{

Variable::Variable(Type type, std::shared_ptr <Identifier> name, std::shared_ptr <Expression> initialization)
	: Declaration(Declaration::Type::Variable), type(type), initialization(initialization)
{
	this->name = name->getValue();
	setToken(name->getToken());
}

bool Variable::validate(Validator& validator)
{
	if(!referredType.has_value())
	{
		if(!Declaration::validate(validator))
		{
			return false;
		}

		if(isAttribute())
		{
			if(type != Type::Local)
			{
				SourceLocation location(validator.getParserContext().source, getToken());
				validator.getParserContext().client.sourceError(location, "Only local variables can be declared as attributes");
				return false;
			}

			if(!initialization.expired())
			{
				SourceLocation location(validator.getParserContext().source, getToken());
				validator.getParserContext().client.sourceError(location, "Attribute declarations cannot have an initializer");
				return false;
			}

			// TODO: Should attribute names have a result type in case they are passed to a function or such?

			// No more validation needed for attributes.
			return true;
		}

		if(initialization.expired())
		{
			SourceLocation location(validator.getParserContext().source, getToken());
			validator.getParserContext().client.sourceError(location, "Expected '='");
			return false;
		}

        // Temporarily refer to void. This is to stop recursive validation.
        // TODO: Should there be an error type or similar to indicate uninitializated variables?
		referredType.emplace(validator.getParserContext().client.getBuiltin().get(Builtin::DataType::Void));

		assert(!initialization.expired());
		auto init = initialization.lock();

		if(type == Type::Parameter)
		{
			if(!validator.traverseExpression(init))
			{
				return false;
			}

			if(!init->getResultType()->isTypeName)
			{
				SourceLocation location(validator.getParserContext().source, init->getToken());
				validator.getParserContext().client.sourceError(location, "Parameters must be initialized with types");
				return false;
			}

			// TODO Maybe here: Make sure that a parameter like "a = type int64" isn't allowed
			// as it doesn't make any sense.

			// While function parameters can be initialized with a raw type name,
			// it doesn't mean that the parameter refers to such. Instead we want
			// to treat it as a promise to the caller that this parameter is just
			// a normal variable of some given type when a function starts execution.
			auto nonTypeName = *init->getResultType();
			nonTypeName.isTypeName = false;

			init->setResultType(nonTypeName);
			referredType.emplace(nonTypeName);

			// Finally validate the parameter name itself for completeness sake.
			if(!validator.traverseExpression(init))
			{
				return false;
			}
		}

		else if(!validator.traverseExpression(init))
		{
			return false;
		}

		// TODO: Should a declaration be referred to in some case?
        assert(init->getResultType());
		referredType.emplace(*init->getResultType());
	}

	return true;
}

std::shared_ptr <Expression> Variable::getInitialization()
{
	assert(!initialization.expired());
	return initialization.lock();
}

const char* Variable::getTypeString(Type type)
{
	switch(type)
	{
		case Type::Generic: return "Generic";
		case Type::Parameter: return "Parameter";
		case Type::Local: return "Local variable";

		default: {}
	}

	return "(declroot) ???";
}

const char* Variable::getTypeString() const
{
	return getTypeString(type);
}

Variable::Root::Root(Variable::Type type)
	: Statement(Statement::Type::VariableRoot), type(type)
{
}

std::weak_ptr <Node> Variable::Root::handleToken(Node::ParserContext& ctx, Token& token)
{
	assert(!initializer);
	initializer = std::make_shared <Expression::Root> ();
	adopt(initializer);
	return initializer->handleToken(ctx, token);
}

std::weak_ptr <Node> Variable::Root::invokedNodeExited(Node::ParserContext& ctx, Token&)
{
	assert(ctx.exitedFrom == initializer);

	if(!initializer->getFirst() && requiresDeclaration(ctx))
	{
		return {};
	}

	// TODO: Initializer instead?
	auto variableRoot = std::static_pointer_cast <Variable::Root> (shared_from_this());

	ArgumentAccessor declarations(variableRoot);
	auto declContainer = getParentWithDeclarationStorage();

	while(auto node = declarations.getNext())
	{
		std::shared_ptr <Expression> nameAt;
		std::shared_ptr <Expression> initialization;

		if(node->getType() == Expression::Type::BinaryOperator)
		{
			// TODO: Don't save the whole binary op but maybe just the left and right nodes.
			auto op = std::static_pointer_cast <BinaryOperator> (node);
			if(op->getType() == BinaryOperator::Type::Assign)
			{
				nameAt = op->getLeft();
				initialization = op->getRight();
			}

			else
			{
				// If a binary operator exists for initialization, syntactically it has to be an assignment.
				SourceLocation location(ctx.source, node->getToken());
				ctx.client.sourceError(location, "Initialization must be done with '='");
				return {};
			}
		}

		else if (node->getType() == Expression::Type::Value)
		{
			nameAt = std::static_pointer_cast <Value> (node);
		}

		// TODO: Support "*foo" for name injection?
		else
		{
			SourceLocation location(ctx.source, node->getToken());
			ctx.client.sourceError(location, "Expected an assignment or an attribute declaration");
			return {};
		}

		assert(nameAt);
		if(nameAt->getToken().getType() != Token::Type::Identifier)
		{
			SourceLocation location(ctx.source, nameAt->getToken());
			ctx.client.sourceError(location, "Expected an identifier");
			return {};
		}

		// If the token type of the name is an identifier, the name should be an Identifier object.
		assert(std::dynamic_pointer_cast <Identifier> (nameAt));
		auto name = std::static_pointer_cast <Identifier> (nameAt);
		auto decl = std::make_shared <Variable> (variableRoot->getType(), name, initialization);

		declContainer->adopt(decl);
		name->setReferred(decl);

		declared.emplace_back(decl);
		declContainer->getDeclarationStorage().add(std::move(decl));
	}

	return getParent();
}

std::shared_ptr <Expression::Root> Variable::Root::getInitializer() const
{
	return initializer;
}

Variable::Type Variable::Root::getType() const
{
	return type;
}

const char* Variable::Root::getTypeString() const
{
	return "Variable root";
}

bool Variable::Root::onInitialize(cap::ParserContext& ctx, bool expectsToken)
{
	// The variable root is valid if a token will follow or a declaration is not required.
	return expectsToken || !requiresDeclaration(ctx);
}

bool Variable::Root::requiresDeclaration(cap::ParserContext& ctx)
{
	std::string_view error;
	switch(type)
	{
		case cap::Variable::Type::Generic: error = "Expected a generic"; break;
		case cap::Variable::Type::Local: error = "Expected a variable"; break;

		// Allow parameters without an initializer to support empty parentheses.
		case cap::Variable::Type::Parameter: break;
	}

	if(!error.empty())
	{
		SourceLocation location(ctx.source, getToken());
		ctx.client.sourceError(location, error.data());
		return true;
	}

	return false;
}

}
