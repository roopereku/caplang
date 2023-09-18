#ifndef CAP_NODE_EXPRESSION_HH
#define CAP_NODE_EXPRESSION_HH

#include <cap/Node.hh>

namespace cap
{

class Expression : public Node
{
public:
	Expression(Token&& token);

	bool isExpression() override
	{
		return true;
	}

	virtual bool isValue();
	virtual bool isOperator();
	virtual bool isVariableDeclaration();
	virtual bool isExpressionRoot();

	static std::shared_ptr <Expression> parseToken(Token&& token, ParserState& state);
};

}

#endif
