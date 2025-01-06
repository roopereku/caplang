#include <cap/Validator.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Value.hh>

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

Traverser::Result Validator::onDeclarationRoot(std::shared_ptr <Declaration::Root> node)
{
	if(!node->getFirst())
	{
		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, "Expected an expression after 'let'");
		return Result::Stop;
	}

	if(!checkAssignment(node->getFirst()))
	{
		return Result::Stop;
	}

	return Result::Exit;
}

Traverser::Result Validator::onBinaryOperator(std::shared_ptr <BinaryOperator> node)
{
	return Result::Continue;
}

bool Validator::checkAssignment(std::shared_ptr <Expression> node)
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
			if(!checkAssignment(op->getLeft()) ||
				!checkAssignment(op->getRight()))
			{
				return false;
			}
		}

		else if(op->getType() == BinaryOperator::Type::Assign)
		{
			// Make sure that the declaration target is valid.
			if(!checkDeclarationTarget(op->getLeft(), false))
			{
				return false;
			}

			// Validate the expression after the assignment.
			if(!traverseExpression(op->getRight()))
			{
				return false;
			}

			// TODO: Make sure that the given declaration target doesn't exist already.

			// TODO: Split the expression into separate Declaration nodes.
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
	if(isIdentifier(node))
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

bool Validator::isIdentifier(std::shared_ptr <Expression> node)
{
	return node->getType() == Expression::Type::Value &&
		node->getToken().getType() == Token::Type::Identifier;
}

}
