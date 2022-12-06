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
	Type(Token* name, bool isPrimitive = false, unsigned baseSize = 0)
		: name(name), isPrimitive(isPrimitive), baseSize(baseSize)
	{}

	bool hasConversion(Type* other);

	Token* name;
	std::shared_ptr <Scope> scope;

	bool isPrimitive = false;
	unsigned baseSize;

	static Type* findPrimitiveType(TokenType t);
	static Type* findPrimitiveType(Token* name);
	static bool isPrimitiveName(Token* name);

private:
	bool isNumeric();
	bool isStringLiteral();
};

}

#endif
