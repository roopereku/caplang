#ifndef CAP_NODE_FUNCTION_CALL_HH
#define CAP_NODE_FUNCTION_CALL_HH

#include <cap/node/OneSidedOperator.hh>
#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class FunctionCall : public OneSidedOperator
{
public:
	FunctionCall(Token&& token) : OneSidedOperator(std::move(token), Type::FunctionCall)
	{
	}

	void setParameters(std::shared_ptr <ExpressionRoot> params)
	{
		adopt(params);
		parameters = params;
	}

	std::shared_ptr <ExpressionRoot> getParameters()
	{
		return parameters;
	}

private:
	std::shared_ptr <ExpressionRoot> parameters;
};

}

#endif
