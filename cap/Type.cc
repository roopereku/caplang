#include "Type.hh"
#include "Debug.hh"

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
	Cap::Type(&primitiveTokens[0], true),	//	i8
	Cap::Type(&primitiveTokens[1], true),	//	u8
	Cap::Type(&primitiveTokens[2], true),	//	i16
	Cap::Type(&primitiveTokens[3], true),	//	u16
	Cap::Type(&primitiveTokens[4], true),	//	i32
	Cap::Type(&primitiveTokens[5], true),	//	u32
	Cap::Type(&primitiveTokens[6], true),	//	i64
	Cap::Type(&primitiveTokens[7], true),	//	u64
};

bool Cap::Type::hasConversion(Type& other)
{
	if(isPrimitive)
	{
		//	TODO add floats
		//	Are both of the types primitives that are numeric
		if(	(this >= &primitives[0] && this <= &primitives[7]) &&
			(&other >= &primitives[0] && &other <= &primitives[7]))
		{
			return true;
		}
	}

	return false;
}

Cap::Type* Cap::Type::findPrimitiveType(TokenType t)
{
	switch(t)
	{
		case TokenType::Integer: return &primitives[6];
		case TokenType::Character: return &primitives[0];

		default:
			DBG_LOG("UNIMPLEMENTED '%s'", Token::getTypeString(t));
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
