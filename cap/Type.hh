#ifndef TYPE_HEADER
#define TYPE_HEADER

#include "Token.hh"
#include "Variable.hh"

#include <vector>
#include <memory>

namespace Cap
{

struct Scope;
struct Type
{
	Type(Token* name, bool isPrimitive = false)
		: name(name), isPrimitive(isPrimitive)
	{}

	Token* name;
	std::shared_ptr <Scope> scope;
	bool isPrimitive = false;

	static Type* findPrimitiveType(TokenType t);
	static Type* findPrimitiveType(Token* name);
	static bool isPrimitiveName(Token* name);
};

}

#endif
