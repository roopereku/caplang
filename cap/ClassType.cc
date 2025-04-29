#include <cap/ClassType.hh>
#include <cap/ParserContext.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

ClassType::ClassType()
	: TypeDefinition(Type::Class, generics)
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
		if(!getParentScope()->declarations.add(ctx, std::static_pointer_cast <ClassType> (shared_from_this())))
		{
			return {};
		}

		return weak_from_this();
	}

	// Parse a generic.
	else if(token.isOpeningBracket(ctx, '<'))
	{
		// TODO: Check for this again in cases like "type T <a> <b>"?

		generic = std::make_shared <Variable::Root> (Variable::Type::Generic);
		adopt(generic);
		return generic;
	}

	// Parse base types.
	else if(ctx.source[token.getIndex()] == ':')
	{
		// TODO: Check for this again in cases like "type T : a : b"?
		// Mostly applies if there is a way to end the first expression.

		baseTypes = std::make_shared <Expression::Root> ();
		adopt(baseTypes);
		return baseTypes;
	}

	else if(!body)
	{
		body = Scope::startParsing(ctx, token, true);
		if(body)
		{
			adopt(body);
		}

		return body;
	}

	assert(false);
	return {};
}

std::weak_ptr <Node> ClassType::invokedNodeExited(ParserContext& ctx, Token&)
{
	if(ctx.exitedFrom == generic)
	{
		ctx.declarationLocation = nullptr;
		return weak_from_this();
	}

	else if(ctx.exitedFrom == baseTypes)
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

std::shared_ptr <Expression::Root> ClassType::getBaseTypeRoot()
{
	return baseTypes;
}

std::shared_ptr <Variable::Root> ClassType::getGenericRoot()
{
	return generic;
}

std::shared_ptr <Scope> ClassType::getBody()
{
	return body;
}

bool ClassType::validate(Validator&)
{
	if(!referredType.has_value())
	{
		referredType = TypeContext(std::static_pointer_cast <ClassType> (shared_from_this()));
		referredType.value().isTypeName = true;
	}

	return true;
}

const char* ClassType::getTypeString() const
{
	return "Class Type";
}

}
