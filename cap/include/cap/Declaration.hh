#ifndef CAP_DECLARATION_HH
#define CAP_DECLARATION_HH

#include <cap/Expression.hh>

namespace cap
{

class Declaration : public Expression::Root
{
public:
	Declaration()
		: Expression::Root(Type::Declaration)
	{
	}
};

}

#endif
