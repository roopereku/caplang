#include "Type.hh"
#include "Debug.hh"
#include "Logger.hh"

#include <array>

//	Names of the primitives
static const char* primitiveNames =
	"i8" "u8" "i16" "u16" "i32" "u32" "i64" "u64" "float" "double" "stringliteral";

//	Since the type name is a Token pointer, we need to create the tokens
static Cap::Token primitiveTokens[]
{
	{ primitiveNames + 0, Cap::TokenType::Identifier, 2, 0, 0 },
	{ primitiveNames + 2, Cap::TokenType::Identifier, 2, 0, 0 },
	{ primitiveNames + 4, Cap::TokenType::Identifier, 3, 0, 0 },
	{ primitiveNames + 7, Cap::TokenType::Identifier, 3, 0, 0 },
	{ primitiveNames + 10, Cap::TokenType::Identifier, 3, 0, 0 },
	{ primitiveNames + 13, Cap::TokenType::Identifier, 3, 0, 0 },
	{ primitiveNames + 16, Cap::TokenType::Identifier, 3, 0, 0 },
	{ primitiveNames + 19, Cap::TokenType::Identifier, 3, 0, 0 },
	{ primitiveNames + 22, Cap::TokenType::Identifier, 5, 0, 0 },
	{ primitiveNames + 27, Cap::TokenType::Identifier, 6, 0, 0 },
	{ primitiveNames + 33, Cap::TokenType::Identifier, 13, 0, 0 }
};

static std::array <Cap::Type, 11> primitives
{
	Cap::Type(&primitiveTokens[0], true, 1),	//	i8
	Cap::Type(&primitiveTokens[1], true, 1),	//	u8
	Cap::Type(&primitiveTokens[2], true, 2),	//	i16
	Cap::Type(&primitiveTokens[3], true, 2),	//	u16
	Cap::Type(&primitiveTokens[4], true, 4),	//	i32
	Cap::Type(&primitiveTokens[5], true, 4),	//	u32
	Cap::Type(&primitiveTokens[6], true, 8),	//	i64
	Cap::Type(&primitiveTokens[7], true, 8),	//	u64
	Cap::Type(&primitiveTokens[8], true, 4),	//	float
	Cap::Type(&primitiveTokens[9], true, 8),	//	double
	Cap::Type(&primitiveTokens[10], true),		//	stringliteral
};

bool Cap::Type::isNumeric()
{
	return this >= &primitives[0] && this <= &primitives[9];
}

bool Cap::Type::isStringLiteral()
{
	return this == &primitives[9];
}

bool Cap::Type::hasConversion(Type* other)
{
	if(other == nullptr)
	{
		Logger::error("FIXME: No conversion because other is nullptr");
		return false;
	}

	if(isPrimitive)
	{
		if(isNumeric())
			return other->isNumeric();

		else if(isStringLiteral())
			return other->isStringLiteral();
	}

	return false;
}

Cap::Type* Cap::Type::findPrimitiveType(TokenType t)
{
	switch(t)
	{
		case TokenType::Binary:
		case TokenType::Integer:
		case TokenType::Hexadecimal:
			return &primitives[6];

		case TokenType::Float: return &primitives[8];
		case TokenType::Double: return &primitives[9];
		case TokenType::Character: return &primitives[0];

		case TokenType::String: return &primitives[9];

		default:
			Logger::error("UNIMPLEMENTED TYPE '%s'", Token::getTypeString(t));
			return nullptr;
	}
}

Cap::Type* Cap::Type::findPrimitiveType(Token* name)
{
	for(auto& it : primitives)
	{
		if(it.name->tokenEquals(name))
			return &it;
	}

	return findPrimitiveType(name->type);
}

bool Cap::Type::isPrimitiveName(Token* name)
{
	for(auto& t : primitives)
	{
		if(name->tokenEquals(t.name))
			return true;
	}

	return false;
}
