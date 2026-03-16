#include <cap/Function.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

Function::Function() :
	Declaration(Declaration::Type::Function, m_parameters)
{
}

std::weak_ptr <Node> Function::handleToken(ParserContext& ctx, Token& token)
{
	// Parse the function name.
	if(m_name.empty())
	{
		// TODO: Check if there is no name but the token is an
		// opening parenthesis, go straight to signature parsing.
		
		if(token.getType() != Token::Type::Identifier)
		{
			SourceLocation location(ctx.m_source, token);
			ctx.m_client.sourceError(location, "Expected an identifier after 'func'");
			return {};
		}

		setToken(token);
		m_name = ctx.m_source.getString(token);

		assert(getParentScope());
		getParentScope()->declarations.add(std::static_pointer_cast <Function> (shared_from_this()));
		return weak_from_this();
	}

	// Parse the function parameters.
	else if(!getParameterRoot())
	{
		if(!token.isOpeningBracket(ctx, '('))
		{
			// TODO: How about anonymous functions?
			SourceLocation location(ctx.m_source, token);
			ctx.m_client.sourceError(location, "Expected '(' after function name");
			return {};
		}

		initializeParameters();
		adopt(getParameterRoot());

		ctx.m_declarationLocation = shared_from_this();
		return getParameterRoot();
	}
	
	// Parse the function return type.
	else if(token.getType() == Token::Type::Operator && ctx.m_source.match(token, L"->"))
	{
		assert(!getReturnTypeRoot());
		initializeReturnType();

		adopt(getReturnTypeRoot());
		return getReturnTypeRoot();
	}

	else if(!m_body)
	{
		m_body = Scope::startParsing(ctx, token, false);

		if(m_body)
		{
			adopt(m_body);
		}

		return m_body;
	}

	assert(false);
	return {};
}

std::weak_ptr <Node> Function::invokedNodeExited(ParserContext& ctx, Token&)
{
	if(ctx.m_exitedFrom == getParameterRoot())
	{
		ctx.m_declarationLocation = nullptr;
		return weak_from_this();
	}

	else if(ctx.m_exitedFrom == getReturnTypeRoot())
	{
		return weak_from_this();
	}

	else if(ctx.m_exitedFrom == m_body)
	{
		return getParent();
	}

	assert(false);
	return {};
}

std::shared_ptr <Scope> Function::getBody() const
{
	return m_body;
}

bool Function::validate(Validator& validator)
{
	if(!m_referredType.has_value())
	{
		if(!Declaration::validate(validator))
		{
			return false;
		}

		m_referredType.emplace(TypeContext(*this));

		if(!getReturnTypeRoot())
		{
			initializeReturnType();
		}

		if(!validator.traverseStatement(getParameterRoot()) ||
			!validator.traverseExpression(getReturnTypeRoot()) ||
			!validator.traverseScope(m_body))
		{
			return false;
		}

		// If no return type still exists, default to void.
		if(!getReturnTypeRoot()->getResultType())
		{
			auto& voidType = validator.getParserContext().m_client.getBuiltin().get(Builtin::DataType::Void);
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
