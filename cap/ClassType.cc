#include <cap/ClassType.hh>
#include <cap/ParserContext.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

ClassType::ClassType()
	: TypeDefinition(Type::Class)
{
}

std::weak_ptr <Node> ClassType::handleToken(ParserContext& ctx, Token& token)
{
	if(name.empty())
	{
		if(token.getType() != Token::Type::Identifier)
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected an identifier after 'type'");
			return {};
		}

		setToken(token);
		name = ctx.source.getString(token);

		assert(getParentScope());
		if(!getParentScope()->addDeclaration(ctx, std::static_pointer_cast <ClassType> (shared_from_this())))
		{
			return {};
		}

		return weak_from_this();
	}

	else if(!body)
	{
		// TODO: Establish an order of what is expected when.

		// Parse a generic.
		if(token.isOpeningBracket(ctx, '<'))
		{
			generic = std::make_shared <Expression::Root> ();
			adopt(generic);
			ctx.implicitDeclaration = Declaration::Root::Type::Generic;
			return generic;
		}

		// Parse base types.
		if(ctx.source[token.getIndex()] == ':')
		{
			baseTypes = std::make_shared <Expression::Root> ();
			adopt(baseTypes);
			return baseTypes;
		}

		// Expect a scope beginning.
		if(!token.isOpeningBracket(ctx, '{'))
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected '{' after a type declaration");
			return {};
		}

		body = std::make_shared <Scope> (true);
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

std::shared_ptr <Expression::Root> ClassType::getBaseTypeRoot()
{
	return baseTypes;
}

std::shared_ptr <Expression::Root> ClassType::getGenericRoot()
{
	return generic;
}

std::shared_ptr <Scope> ClassType::getBody()
{
	return body;
}

bool ClassType::validate(Validator&)
{
	if(!referredType.getReferenced())
	{
		referredType = TypeContext(std::static_pointer_cast <ClassType> (shared_from_this()));
		referredType.isTypeName = true;
	}

	return true;
}

const char* ClassType::getTypeString() const
{
	return "Class Type";
}

}
