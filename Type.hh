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
	Type(Token* name) : name(name) {}

	Token* name;
	std::shared_ptr <Scope> scope;
};

}

#endif
