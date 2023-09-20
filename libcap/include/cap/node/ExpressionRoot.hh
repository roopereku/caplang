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

	bool isExpressionRoot() override
	{
		return true;
	}

	std::shared_ptr <Expression> getRoot()
	{
		return root;
	}

private:
	/// Replaces the root with the given operator node.
	bool replaceExpression(std::shared_ptr <Expression> node) override;

	//bool handleExpressionNode(std::shared_ptr <Expression> node) override;

	std::shared_ptr <Expression> root;
};

}

#endif
