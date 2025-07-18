#include <cap/Variable.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Validator.hh>
#include <cap/Value.hh>
#include <cap/Identifier.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

Variable::Variable(Type type, std::weak_ptr <BinaryOperator> initialization) :
	Declaration(Declaration::Type::Variable),
	type(type), initialization(initialization)
{
	assert(!initialization.expired());
	auto node = initialization.lock();
	assert(node->getType() == BinaryOperator::Type::Assign);

	assert(node->getLeft()->getType() == Expression::Type::Value);
	auto value = std::static_pointer_cast <Value> (node->getLeft());
	assert(value->getType() == Value::Type::Identifier);
	auto nameIdentifier = std::static_pointer_cast <Identifier> (node->getLeft());

	name = nameIdentifier->getValue();
	setToken(nameIdentifier->getToken());
}

bool Variable::validate(Validator& validator)
{
	if(!referredType.has_value())
	{
        // Temporarily refer to void. This is to stop recursive validation.
        // TODO: Should there be an error type or similar to indicate uninitializated variables?
		referredType.emplace(validator.getParserContext().client.getBuiltin().get(Builtin::DataType::Void));

		assert(!initialization.expired());
		auto init = initialization.lock();

		if(type == Type::Parameter)
		{
			// Avoid calling Validator::onBinaryOperator as it would be unhappy
			// with the right side operand resulting in a raw typename.
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

			// TODO Maybe here: Make sure that a parameter like "a = type int64" isn't allowed
			// as it doesn't make any sense.

			// While function parameters can be initialized with a raw type name,
			// it doesn't mean that the parameter refers to such. Instead we want
			// to treat it as a promise to the caller that this parameter is just
			// a normal variable of some given type when a function starts execution.
			auto nonTypeName = *init->getRight()->getResultType();
			nonTypeName.isTypeName = false;

			init->setResultType(nonTypeName);
			referredType.emplace(nonTypeName);

			// Finally validate the parameter name itself for completeness sake.
			if(!validator.traverseExpression(init->getLeft()))
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
	return initialization.lock()->getRight();
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
		if(node->getType() == Expression::Type::BinaryOperator)
		{
			auto op = std::static_pointer_cast <BinaryOperator> (node);
			if(op->getType() == BinaryOperator::Type::Assign)
			{
				if(op->getLeft()->getToken().getType() != Token::Type::Identifier)
				{
					SourceLocation location(ctx.source, op->getLeft()->getToken());
					ctx.client.sourceError(location, "Expected an identifier");
					return {};
				}

				auto decl =  std::make_shared <Variable> (variableRoot->getType(), op);

				// TODO: Something like ArgumentAccessor::getNextIdentifier might be useful.
				assert(op->getLeft()->getType() == Expression::Type::Value);
				auto value = std::static_pointer_cast <Value> (op->getLeft());
				assert(value->getType() == Value::Type::Identifier);
				auto name = std::static_pointer_cast <Identifier> (value);

				declContainer->adopt(decl);
				name->setReferred(decl);

				declContainer->getDeclarationStorage().add(std::move(decl));
				continue;
			}
		}

		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, "Expected '='");
		return {};
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
