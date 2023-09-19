#ifndef CAP_NODE_EXPRESSION_HH
#define CAP_NODE_EXPRESSION_HH

#include <cap/Node.hh>

namespace cap
{

class Operator;

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

private:
	static std::shared_ptr <Operator> parseOneSidedOperator(Token&& token, ParserState& state);
	static std::shared_ptr <Operator> parseTwoSidedOperator(Token&& token, ParserState& state);
};

}

#endif
