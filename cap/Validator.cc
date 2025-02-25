#include <cap/Validator.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Value.hh>
#include <cap/Variable.hh>
#include <cap/PrimitiveType.hh>

#include <cassert>

namespace cap
{

Validator::Validator(ParserContext& ctx)
	: ctx(ctx)
{
}

void Validator::onNodeExited(std::shared_ptr <Node>, Result)
{
}

Traverser::Result Validator::onFunction(std::shared_ptr <Function> node)
{
	if(!node->getSignature()->validate(*this))
	{
		return Result::Stop;
	}

	auto scope = node->getParentScope();

	// Functions only care about duplicate names or other function in the same scope.
	for(auto decl : scope->iterateDeclarations())
	{
		// Make sure that whatever is being matched against is validated.
		if(!decl->validate(*this))
		{
			return Result::Stop;
		}

		if(decl->getName() == node->getName() && decl != node)
		{
			// If the declaration with the same name is a function,
			// make sure that the parameters aren't identical.
			if(decl->getType() == Declaration::Type::Function)
			{
				auto function = std::static_pointer_cast <Function> (decl);
				auto nodeParams = node->getSignature()->getParameterRoot();

				auto [compatible, unidentical] = function->getSignature()->matchParameters(nodeParams);
				if(compatible && unidentical == 0)
				{
					// TODO: Give more context for the existing function?
					SourceLocation location(ctx.source, node->getToken());
					ctx.client.sourceError(location, "Function with the same parameters already exists");
					return Result::Stop;
				}
			}

			// Other declarations of the same name aren't allowed.
			else
			{
				// TODO: Something here?
				return Result::Stop;
			}
		}
	}

	if(!traverseScope(node->getBody()))
	{
		return Result::Stop;
	}

	return Result::Exit;
}

Traverser::Result Validator::onClassType(std::shared_ptr <ClassType> node)
{
	if(node->getBaseTypeRoot() &&
		!traverseExpression(node->getBaseTypeRoot()))
	{
		return Result::Stop;
	}

	auto scope = node->getParentScope();
	return Result::Continue;
}

Traverser::Result Validator::onExpressionRoot(std::shared_ptr <Expression::Root> node)
{
	if(node->getFirst())
	{
		if(!traverseExpression(node->getFirst()))
		{
			return Result::Stop;
		}

		node->setResultType(node->getFirst()->getResultType());
	}

	return Result::Exit;
}

Traverser::Result Validator::onDeclarationRoot(std::shared_ptr <Declaration::Root> node)
{
	if(!node->getFirst())
	{
		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, "Expected an expression after 'let'");
		return Result::Stop;
	}

	return Result::Continue;
}

Traverser::Result Validator::onBinaryOperator(std::shared_ptr <BinaryOperator> node)
{
	if(!traverseExpression(node->getLeft()) ||
		!traverseExpression(node->getRight()))
	{
		return Result::Stop;
	}

	// Saving the result type doesn't make sense for commas.
	if(node->getType() != BinaryOperator::Type::Comma)
	{
		// TODO: Check if there is a binary operator overload and set the result
		// type based on that.
		node->getLeft()->setResultType(node->getRight()->getResultType());
		node->setResultType(node->getLeft()->getResultType());
	}

	return Result::Exit;
}

Traverser::Result Validator::onBracketOperator(std::shared_ptr <BracketOperator> node)
{
	if(node->getType() != BracketOperator::Type::Call)
	{
		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, "TODO: Bracket operator validation implemented for call operators only");
		return Result::Stop;
	}

	if(node->getContext()->getType() == Expression::Type::Value)
	{
		auto value = std::static_pointer_cast <Value> (node->getContext());
		if(value->getToken().getType() != Token::Type::Identifier)
		{
			SourceLocation location(ctx.source, value->getToken());
			ctx.client.sourceError(location, "Literal values cannot be called");
			return Result::Stop;
		}
	}

	// First validate the parameters so that they can be used for matching.
	if(!traverseExpression(node->getInnerRoot()))
	{
		return Result::Stop;
	}

	associatedParameters = node->getInnerRoot();
	if(!traverseExpression(node->getContext()))
	{
		return Result::Stop;
	}

	auto callable = node->getContext()->getResultType().getReferenced();
	assert(callable);
	assert(callable->getType() == TypeDefinition::Type::Callable);

	// The call operator now results in the return type of the callable.
	auto returnTypeRoot = std::static_pointer_cast <CallableType> (callable)->getReturnTypeRoot();
	assert(returnTypeRoot);
	node->setResultType(returnTypeRoot->getResultType());

	assert(node->getResultType().getReferenced());
	return Result::Exit;
}

Traverser::Result Validator::onValue(std::shared_ptr <Value> node)
{
	// TODO: Handle scope context change when binary operator encounters ".".
	if(node->getToken().getType() == Token::Type::Identifier)
	{
		auto parentFunction = node->getParentFunction();
		auto scope = parentFunction ? parentFunction->getBody() : node->getParentScope();

		size_t triedCallables = 0;

		// Recursion might be problematic when the associated parameters
		// are stored beyond this point.
		auto parameters = associatedParameters;
		associatedParameters = nullptr;

		for(auto decl : scope->recurseDeclarations())
		{
			if(decl->getName() != node->getValue())
			{
				continue;
			}

			// Make sure that whatever is being matched against is validated.
			if(!decl->validate(*this))
			{
				return Result::Stop;
			}

			// Should parameters be matched as well?
			if(parameters)
			{
				std::shared_ptr <CallableType> callable;

				switch(decl->getType())
				{
					// Normal function calls.
					case Declaration::Type::Function:
					{
						callable = std::static_pointer_cast <Function> (decl)->getSignature();
						break;
					}

					// Constructor calls and type conversions.
					case Declaration::Type::TypeDefinition:
					{
						SourceLocation location(ctx.source, node->getToken());
						ctx.client.sourceError(location, "TODO: Find a constructor or a type conversion");
						return Result::Stop;
					}

					// Operator overload call from an object.
					case Declaration::Type::Variable:
					{
						SourceLocation location(ctx.source, node->getToken());
						ctx.client.sourceError(location, "TODO: Find a callable in the context of an object");
						return Result::Stop;
					}
				}

				assert(callable);
				triedCallables++;

				// TODO: Store the currently most fitting candidate based on unidentical
				// parameters select the most fitting one. This way more fitting function overloads
				// can be prioritized over those where parameters are implicitly casted.

				auto [compatible, unidentical] = callable->matchParameters(parameters);
				if(compatible)
				{
					node->setReferred(decl);
					break;
				}
			}

			else
			{
				node->setReferred(decl);
				break;
			}
		}

		// If nothing was found, check if the value refers to a primitive type.
		if(!node->getReferred())
		{
			auto primitive = PrimitiveType::matchName(ctx.source, node->getToken());

			if(primitive && primitive->validate(*this))
			{
				TypeContext resultType(primitive);
				resultType.isTypeName = true;
				node->setReferred(primitive);

			}
		}

		if(!node->getReferred())
		{
			// If we tried to match callables, show a different message.
			// TODO: node->getValue() should also be something else in the
			// case of type constructors or type conversions.
			const char* msg = triedCallables ?
				"No matching overload found for" :
				"Undeclared identifier";

			SourceLocation location(ctx.source, node->getToken());
			ctx.client.sourceError(location, msg, " '", node->getValue(), '\'');
			return Result::Stop;
		}
	}

	else
	{
		// Make the non-identifier value refer to a primitive type.
		// TODO: Use PrimitiveType::matchValue when it replaces matchToken.
		node->setResultType(TypeContext(PrimitiveType::matchToken(node->getToken())));
		assert(node->getResultType().getReferenced());
	}

	return Result::Exit;
}

}
