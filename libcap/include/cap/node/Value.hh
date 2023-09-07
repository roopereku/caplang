#ifndef CAP_NODE_VALUE_HH
#define CAP_NODE_VALUE_HH

#include <cap/node/Expression.hh>

namespace cap
{

class Value : public Expression
{
public:
	Value(Token&& token) : Expression(std::move(token))
	{
	}

	bool isValue() override;
};

}

#endif
