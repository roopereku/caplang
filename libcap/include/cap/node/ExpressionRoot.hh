#ifndef CAP_NODE_EXPRESSION_ROOT_HH
#define CAP_NODE_EXPRESSION_ROOT_HH

#include <cap/node/Expression.hh>

namespace cap
{

class ExpressionRoot : public Expression
{
public:
	ExpressionRoot(Token&& token) : Expression(std::move(token))
	{
	}

	bool handleToken(Token&& token, ParserState& state) override;

private:
	std::shared_ptr <Expression> root;
};

}

#endif
