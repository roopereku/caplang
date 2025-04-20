#include <cap/Function.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

Function::Function()
	: Declaration(Type::Function), signature(std::make_shared <CallableType> ())
{
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

		assert(getParentScope());
		if(!getParentScope()->addDeclaration(ctx, std::static_pointer_cast <Function> (shared_from_this())))
		{
			return {};
		}

		return weak_from_this();
	}

	else if(!signature->getParameterRoot())
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
		adopt(signature->getParameterRoot());

		// Initialize the body so that parameters can be added into it.
		body = std::make_shared <Scope> (false);

		ctx.implicitDeclaration.emplace(Variable::Type::Parameter);
		ctx.declarationLocation = body;

		ctx.delegateFinalBrace = ')';
		return signature->getParameterRoot();
	}

	// End of parameters.
	else if(token.isClosingBracket(ctx, ')'))
	{
		DBG_MESSAGE(ctx.client, "End of params for ", name);
		ctx.braceHasToBeOpener = true;
		ctx.declarationLocation = nullptr;

		return weak_from_this();
	}

	// Parse return types.
	else if(token.getType() == Token::Type::Operator && ctx.source.match(token, L"->"))
	{
		assert(!signature->getReturnTypeRoot());
		signature->initializeReturnType();

		adopt(signature->getReturnTypeRoot());
		return signature->getReturnTypeRoot();
	}

	else
	{
		// Return to the parent node upon a closing brace.
		if(token.isClosingBracket(ctx, '}'))
		{
			if(!ctx.braceHasToBeOpener)
			{
				assert(!getParent().expired());
				return getParent();
			}
		}

		// Expect a scope beginning.
		if(!token.isOpeningBracket(ctx, '{'))
		{
			assert(ctx.braceHasToBeOpener);
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected '{' after a function declaration");
			return {};
		}

		ctx.braceHasToBeOpener = false;
		adopt(body);
		return body;
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

bool Function::validate(Validator& validator)
{
	if(!referredType.has_value())
	{
		referredType.emplace(TypeContext(signature));
		referredType.value().isTypeName = true;

		if(!signature->validate(validator) ||
			!validator.traverseScope(body))
		{
			return false;
		}
	}

	return true;
}

const char* Function::getTypeString() const
{
	return "Function";
}

}
