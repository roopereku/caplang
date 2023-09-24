#ifndef CAP_NODE_RETURN_HH
#define CAP_NODE_RETURN_HH

#include <cap/node/Statement.hh>
#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class Return : public Statement
{
public:
	Return(Token&& token) : Statement(std::move(token))
	{
	}

	bool isReturn() override
	{
		return true;
	}

	std::shared_ptr <ExpressionRoot> expression;
};

}

#endif
