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

void Validator::onNodeExited(std::shared_ptr <Node> node, Result result)
{
}

Traverser::Result Validator::onFunction(std::shared_ptr <Function> node)
{
	auto scope = node->getParentScope();
	if(!checkDeclaration(scope, node))
	{
		return Result::Stop;
	}

	scope->addDeclaration(node);
	return Result::Continue;
}

Traverser::Result Validator::onClassType(std::shared_ptr <ClassType> node)
{
	if(node->getBaseTypeRoot() &&
		!traverseExpression(node->getBaseTypeRoot()))
	{
		return Result::Stop;
	}

	auto scope = node->getParentScope();
	if(!checkDeclaration(scope, node))
	{
		return Result::Stop;
	}

	scope->addDeclaration(node);
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

	if(!checkAssignment(node->getFirst(), node->findTargetScope()))
	{
		return Result::Stop;
	}

	return Result::Exit;
}

Traverser::Result Validator::onBinaryOperator(std::shared_ptr <BinaryOperator> node)
{
	if(!traverseExpression(node->getLeft()) ||
		!traverseExpression(node->getRight()))
	{
		return Result::Stop;
	}

	// TODO: Check if there is a binary operator overload and set the result
	// type based on that.
	node->setResultType(node->getLeft()->getResultType());
	assert(node->getLeft()->getResultType().getReferenced());

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

	associatedParameters = nullptr;
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
		auto scope = node->getParentScope();
		size_t triedCallables = 0;

		for(auto decl : scope->recurseDeclarations())
		{
			if(decl->getName() != node->getValue())
			{
				continue;
			}

			// Should parameters be matched as well?
			if(associatedParameters)
			{
				std::shared_ptr <CallableType> callable;

				DBG_MESSAGE(ctx.client, "Matching parameters for ", decl->getTypeString() , " '", decl->getName(), "'");

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

				// TODO: Store the currently most fitting candidate and based on future unidentical
				// parameters select a more fitting one. This way more fitting function overloads
				// can be prioritized over those where parameters are implicitly casted.

				auto [compatible, unidentical] = callable->matchParameters(associatedParameters);
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

			if(primitive)
			{
				TypeContext resultType(primitive);
				resultType.isTypeName = true;
				node->setReferred(primitive);

				// TODO: The result type is not yet stored in the primitive itself so just calling
				// setReferred isn't enough. This could change when primitives are defined as cap source code.
				node->setResultType(resultType);
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

bool Validator::checkAssignment(std::shared_ptr <Expression> node, std::shared_ptr <Scope> target)
{
	// TODO: Allow something like foo(let a) where the first parameter of function
	// foo takes a parameter that is an output.

	if(node->getType() != Expression::Type::BinaryOperator)
	{
		// The input might be something like "let a". If the current node
		// that should be an assignment or a comma is the target value before
		// the operator, guide the user to add the valid operator.
		checkDeclarationTarget(node, true);
		return false;
	}

	else
	{
		auto op = std::static_pointer_cast <BinaryOperator> (node);

		// Assignments can be separated by commas.
		if(op->getType() == BinaryOperator::Type::Comma)
		{
			if(!checkAssignment(op->getLeft(), target) ||
				!checkAssignment(op->getRight(), target))
			{
				return false;
			}
		}

		else if(op->getType() == BinaryOperator::Type::Assign)
		{
			// Make sure that the declaration target is an identifier.
			if(!checkDeclarationTarget(op->getLeft(), false))
			{
				return false;
			}

			// Validate the expression after the assignment.
			if(!traverseExpression(op->getRight()))
			{
				return false;
			}

			if(!checkDeclaration(target, op->getLeft()))
			{
				return false;
			}

			// TODO: Add a different kind of declaration depending on the initialization type.

			// The lhs of an assignment is known to be a value at this point.
			auto value = std::static_pointer_cast <Value> (op->getLeft());
			target->addDeclaration(std::make_shared <Variable> (value));

			// TODO: Set referred for the declaration?
			// Set the result type for the declaration node and the assignment.
			// The assignment is useful for parameter matching.
			value->setResultType(op->getRight()->getResultType());
			op->setResultType(value->getResultType());
		}

		else
		{
			SourceLocation location(ctx.source, node->getToken());
			ctx.client.sourceError(location, "Expected '=' after a declaration");
			return false;
		}
	}

	return true;
}

bool Validator::checkDeclarationTarget(std::shared_ptr <Expression> node, bool onlyValue)
{
	if(isValueAndIdentifier(node))
	{
		// If only a value without "=" was found, log an error.
		if(onlyValue)
		{
			SourceLocation location(ctx.source, node->getToken());
			ctx.client.sourceError(location, "Missing initialization for '",
				ctx.source.getString(node->getToken()), "'. Add '=' after it");

			return false;
		}

		return true;
	}

	// TODO: If the input is something like "let 5 + 5 = 10", extend the token
	// to cover the entire lhs expression to give the user more context.

	// TODO: If the input is something like "let a.b = 10", add a custom message
	// that points out how the syntax is invalid.

	SourceLocation location(ctx.source, node->getToken());
	ctx.client.sourceError(location, "Expected an identifier before '='");
	return false;
}

bool Validator::checkDeclaration(std::shared_ptr <Scope> scope, std::shared_ptr <Node> name)
{
	auto existing = scope->findDeclaration(ctx.source, name->getToken());

	// If nothing was found yet, the name might represent something builtin.
	if(!existing)
	{
		existing = PrimitiveType::matchName(ctx.source, name->getToken());
	}

	if(existing)
	{
		// TODO: Indicate where the existing declaration was declared.
		SourceLocation location(ctx.source, name->getToken());
		ctx.client.sourceError(location, "'", ctx.source.getString(name->getToken()), "' already exists");
		return false;
	}

	return true;
}

bool Validator::isValueAndIdentifier(std::shared_ptr <Expression> node)
{
	return node->getType() == Expression::Type::Value &&
		node->getToken().getType() == Token::Type::Identifier;
}

}
