#include <cap/PrimitiveType.hh>

#include <array>

namespace cap
{

static std::array <std::shared_ptr <PrimitiveType>, 9> primitives
{
	std::make_shared <PrimitiveType> (L"uint8", 1),
	std::make_shared <PrimitiveType> (L"uint16", 2),
	std::make_shared <PrimitiveType> (L"uint32", 4),
	std::make_shared <PrimitiveType> (L"uint64", 8),

	std::make_shared <PrimitiveType> (L"int8", 1),
	std::make_shared <PrimitiveType> (L"int16", 2),
	std::make_shared <PrimitiveType> (L"int32", 4),
	std::make_shared <PrimitiveType> (L"int64", 8),

	std::make_shared <PrimitiveType> (L"string", 8),
};

PrimitiveType::PrimitiveType(std::wstring_view name, size_t bytes)
	: TypeDefinition(Type::Primitive)
{
	this->name = name;
}

std::shared_ptr <PrimitiveType> PrimitiveType::matchName(Source& source, Token token)
{
	for(auto& t : primitives)
	{
		if(source.match(token, t->name))
		{
			return t;
		}
	}

	return nullptr;
}

std::shared_ptr <PrimitiveType> PrimitiveType::matchToken(Token token)
{
	// TODO: matchToken could be matchValue instead which can return the
	// appropriate type depending on a more concrete value.
	// If the Value node had more specific deriving classes such as IntValue,
	// finding the correct size would be much easier.
	switch(token.getType())
	{
		case Token::Type::Integer:
		case Token::Type::Hexadecimal:
		case Token::Type::Binary:
		case Token::Type::Octal:
		{
			// int64.
			return primitives[7];
		}

		case Token::Type::String:
		{
			// string.
			return primitives[8];
		}

		default: return nullptr;
	}
}

bool PrimitiveType::validate(Validator& validator)
{
	referredType = TypeContext(std::static_pointer_cast <PrimitiveType> (shared_from_this()));
	referredType.isTypeName = true;

	return true;
}

}
