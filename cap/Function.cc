#include <cap/Function.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

Function::Function()
	: Declaration(Type::Function), signature(std::make_shared <CallableType> ())
{
	referredType = TypeContext(signature);
	referredType.isTypeName = true;
}

std::weak_ptr <Node> Function::handleToken(ParserContext& ctx, Token& token)
{
	if(name.empty())
	{
		// TODO: Check if there is no name but the token is an
		// opening parenthesis, go straight to signature parsing.
		
		if(token.getType() != Token::Type::Identifier)
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected an identifier after 'func'");
			return {};
		}

		setToken(token);
		name = ctx.source.getString(token);
		return weak_from_this();
	}

	else if(!signature->getParameters())
	{
		if(!token.isOpeningBracket(ctx, '('))
		{
			// TODO: How about anonymous functions?
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected '(' after function name");
			return {};
		}

		// TODO: Deallocate parameters if none are given.
		signature->initializeParameters();
		adopt(signature->getParameters());

		ctx.implicitDeclaration = Declaration::Root::Type::Parameter;
		return signature->getParameters();
	}

	else if(!body)
	{
		if(signature->getParameters()->getFirst())
		{
			// The implicit declaration should declare parameters.
			assert(signature->getParameters()->getFirst()->getType() == Expression::Type::DeclarationRoot);
			assert(std::static_pointer_cast <Declaration::Root>
					(signature->getParameters()->getFirst())->getType() == Declaration::Root::Type::Parameter);
		}

		// Parse return types.
		if(token.getType() == Token::Type::Operator &&
			ctx.source.match(token, L"->"))
		{
			assert(!signature->getReturnType());
			signature->initializeReturnType();

			adopt(signature->getReturnType());
			return signature->getReturnType();
		}

		// Expect a scope beginning.
		if(!token.isOpeningBracket(ctx, '{'))
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected '{' after a function declaration");
			return {};
		}

		body = std::make_shared <Scope> (false);
		adopt(body);

		return body;
	}

	// Return to the parent node upon a closing brace.
	if(token.isClosingBracket(ctx, '}'))
	{
		assert(!getParent().expired());
		return getParent();
	}

	assert(false);
	return {};
}

std::shared_ptr <CallableType> Function::getSignature() const
{
	return signature;
}

std::shared_ptr <Scope> Function::getBody() const
{
	return body;
}

}
