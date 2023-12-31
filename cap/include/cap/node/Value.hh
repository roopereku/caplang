#ifndef CAP_NODE_VALUE_HH
#define CAP_NODE_VALUE_HH

#include <cap/node/Expression.hh>

namespace cap
{

class Value : public Expression
{
public:
	Value(Token token) : Expression(Expression::Type::Value, token)
	{
	}

	const char* getTypeString() override
	{
		return "Value";
	}
};

}

#endif
