#ifndef CAP_NODE_FUNCTION_CALL_HH
#define CAP_NODE_FUNCTION_CALL_HH

#include <cap/node/OneSidedOperator.hh>

namespace cap
{

class FunctionCall : public OneSidedOperator
{
public:
	FunctionCall(Token&& token) : OneSidedOperator(std::move(token), Type::FunctionCall)
	{
	}
};

}

#endif
