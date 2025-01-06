#ifndef CAP_DECLARATION_HH
#define CAP_DECLARATION_HH

#include <cap/Expression.hh>

namespace cap
{

class Declaration
{
public:
	class Root;
};

class Declaration::Root : public Expression::Root
{
public:
	Root() : Expression::Root(Type::DeclarationRoot)
	{
	}
};

}

#endif
