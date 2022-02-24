#include "Type.hh"

//	A really clear string that contains names for primitives
static const char* primitiveNames = "i8u8i16u16i32u32i64u64";

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
	{ primitiveNames + 19, Cap::TokenType::Identifier, 3, 0, 0 }
};

static Cap::Type primitives[]
{
	Cap::Type(&primitiveTokens[0], true),
	Cap::Type(&primitiveTokens[1], true),
	Cap::Type(&primitiveTokens[2], true),
	Cap::Type(&primitiveTokens[3], true),
	Cap::Type(&primitiveTokens[4], true),
	Cap::Type(&primitiveTokens[5], true),
	Cap::Type(&primitiveTokens[6], true),
	Cap::Type(&primitiveTokens[7], true),
};

Cap::Type* Cap::Type::findPrimitiveType(Token* name)
{
	for(auto& it : primitives)
	{
		if(it.name->tokenEquals(name))
			return &it;
	}

	return nullptr;
}

bool Cap::Type::isPrimitiveName(Token* name)
{
	if(*name->begin == 'i')
	{
		return 	name->stringEquals("i8") ||
				name->stringEquals("i16") ||
				name->stringEquals("i32") ||
				name->stringEquals("i64");

	}

	else if(*name->begin == 'u')
	{
		return	name->stringEquals("u8") ||
				name->stringEquals("u16") ||
				name->stringEquals("u32") ||
				name->stringEquals("u64");
	}

	return false;
}
