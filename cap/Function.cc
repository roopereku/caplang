#include <cap/Function.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

Function::Function() :
	Declaration(Declaration::Type::Function, parameters)
{
}

std::weak_ptr <Node> Function::handleToken(ParserContext& ctx, Token& token)
{
	// Parse the function name.
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
		if(!getParentScope()->declarations.add(ctx, std::static_pointer_cast <Function> (shared_from_this())))
		{
			return {};
		}

		return weak_from_this();
	}

	// Parse the function parameters.
	else if(!getParameterRoot())
	{
		if(!token.isOpeningBracket(ctx, '('))
		{
			// TODO: How about anonymous functions?
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected '(' after function name");
			return {};
		}

		initializeParameters();
		adopt(getParameterRoot());

		ctx.declarationLocation = shared_from_this();
		return getParameterRoot();
	}
	
	// Parse the function return type.
	else if(token.getType() == Token::Type::Operator && ctx.source.match(token, L"->"))
	{
		assert(!getReturnTypeRoot());
		initializeReturnType();

		adopt(getReturnTypeRoot());
		return getReturnTypeRoot();
	}

	else if(!body)
	{
		body = Scope::startParsing(ctx, token, false);

		if(body)
		{
			adopt(body);
		}

		return body;
	}

	assert(false);
	return {};
}

std::weak_ptr <Node> Function::invokedNodeExited(ParserContext& ctx, Token&)
{
	if(ctx.exitedFrom == getParameterRoot())
	{
		ctx.declarationLocation = nullptr;
		return weak_from_this();
	}

	else if(ctx.exitedFrom == getReturnTypeRoot())
	{
		return weak_from_this();
	}

	else if(ctx.exitedFrom == body)
	{
		return getParent();
	}

	assert(false);
	return {};
}

std::shared_ptr <Scope> Function::getBody() const
{
	return body;
}

bool Function::validate(Validator& validator)
{
	if(!referredType.has_value())
	{
		referredType.emplace(TypeContext(*this));

		if(!getReturnTypeRoot())
		{
			initializeReturnType();
		}

		if(!validator.traverseStatement(getParameterRoot()) ||
			!validator.traverseExpression(getReturnTypeRoot()) ||
			!validator.traverseScope(body))
		{
			return false;
		}

		// If no return type still exists, default to void.
		if(!getReturnTypeRoot()->getResultType())
		{
			auto& voidType = validator.getParserContext().client.getBuiltin().getVoidType();
			getReturnTypeRoot()->setResultType(voidType);
		}
	}

	return true;
}

const char* Function::getTypeString() const
{
	return "Function";
}

}
