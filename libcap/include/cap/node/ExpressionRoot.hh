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
	Type& getResultType() override;

	bool isExpressionRoot() override
	{
		return true;
	}

	std::shared_ptr <Expression> getRoot()
	{
		return root;
	}

	void setRoot(std::shared_ptr <Expression>&& node)
	{
		root = std::move(node);
	}

private:
	/// Replaces the root with the given operator node.
	bool replaceExpression(std::shared_ptr <Expression> node) override;

	bool handleExpressionNode(std::shared_ptr <Expression> node, ParserState& state) override;

	std::shared_ptr <Expression> root;
};

}

#endif
