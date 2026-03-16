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

Variable::Variable(Type type, std::shared_ptr <Identifier> name, std::shared_ptr <BinaryOperator> initialization)
	: Declaration(Declaration::Type::Variable), type(type), initialization(initialization)
{
	this->name = name->getValue();
	setToken(name->getToken());
}

bool Variable::validate(Validator& validator)
{
	if(!referredType.has_value())
	{
        // Temporarily refer to void. This is to stop recursive validation.
        // TODO: Should there be an error type or similar to indicate uninitializated variables?
		referredType.emplace(validator.getParserContext().client.getBuiltin().get(Builtin::DataType::Void));

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

			// Some type needs to be associated. Use the "attribute" declaration.
			referredType.emplace(validator.getParserContext().client.getBuiltin().getTypeForAttributeDefinition());

			// No more validation needed for attributes.
			return true;
		}

		if(initialization.expired())
		{
			SourceLocation location(validator.getParserContext().source, getToken());
			validator.getParserContext().client.sourceError(location, "Expected '='");
			return false;
		}

		assert(!initialization.expired());
		auto init = initialization.lock();

		if(type == Type::Parameter)
		{
			// First validate the expression for the initializer value.
			if(!validator.traverseExpression(init->getRight()))
			{
				return false;
			}

			if(!init->getRight()->getResultType()->isTypeName)
			{
				SourceLocation location(validator.getParserContext().source, init->getRight()->getToken());
				validator.getParserContext().client.sourceError(location, "Parameters must be initialized with types");
				return false;
			}

			// While function parameters must be initialized with direct type names, the parameter
			// doesn't actually hold the type itself but just hints that it accepts a value of that type.
			auto nonTypeName = *init->getRight()->getResultType();
			nonTypeName.isTypeName = false;

			init->setResultType(nonTypeName);
			init->getLeft()->setResultType(*init->getResultType());
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
	if (!initialization.expired())
	{
		assert(!isAttribute());
		return initialization.lock()->getRight();
	}

	assert(isAttribute());
	return nullptr;
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

	if(hasAttributes())
	{
		// TODO:
		// In the following case the concept of an attribute range doesn't work.
		//
		// @someAttr
		// let @attr1 a = 10, @attr2 b = 20
		//
		// b cannot point to both someAttr and attr2 without pointing to attr1.

		SourceLocation location(ctx.source, getToken());
		ctx.client.sourceError(location, "TODO: Figure out if attributes in Variable::Root are allowed");
		return {};
	}

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
		std::shared_ptr <BinaryOperator> initialization;

		if(node->getType() == Expression::Type::BinaryOperator)
		{
			initialization = std::static_pointer_cast <BinaryOperator> (node);
			if(initialization->getType() != BinaryOperator::Type::Assign)
			{
				// If a binary operator exists for initialization, syntactically it has to be an assignment.
				SourceLocation location(ctx.source, node->getToken());
				ctx.client.sourceError(location, "Initialization must be done with '='");
				return {};
			}

			nameAt = initialization->getLeft();
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

		// TODO: We need to append if attributes are inherited from some parent context.
		// The declaration uses the same attributes as the name.
		decl->setAttributeRange(name->getAttributeRange());

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
