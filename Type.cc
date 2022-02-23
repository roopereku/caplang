#include "Type.hh"

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
