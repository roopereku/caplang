#include <cap/ModifierRoot.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>

#include <array>

namespace cap
{
ModifierRoot::ModifierRoot(Type type)
	: Expression::Root(Expression::Type::ModifierRoot), type(type)
{
}

std::shared_ptr <ModifierRoot> ModifierRoot::create(ParserContext& ctx, Token token)
{
	std::array <std::wstring_view, 1> modifiers
	{
		L"type"
	};

	for(size_t i = 0; i < modifiers.size(); i++)
	{
		if(ctx.source.match(token, modifiers[i]))
		{
			return std::make_shared <ModifierRoot> (static_cast <Type> (i));
		}
	}
	
	return nullptr;
}

ModifierRoot::Type ModifierRoot::getType() const
{
	return type;
}

const char* ModifierRoot::getTypeString(Type type)
{
	switch(type)
	{
		case Type::Alias: return "Alias";
	}

	return "(modifierroot) ???";
}

const char* ModifierRoot::getTypeString() const
{
	return getTypeString(type);
}

}
