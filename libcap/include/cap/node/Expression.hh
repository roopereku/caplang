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

	/// Custom handler for when an operator should replace what's currently stored.
	///
	/// \param The node that will replace the currently stored node.
	/// \return True if replacing succeeded.
	virtual bool replaceExpression(std::shared_ptr <Expression> node);

	/// Convert a token into the appropriate expression node.
	///
	/// \param token The token converted into a node.
	/// \param The current state of the parser.
	/// \returns The constructed node or nullptr.
	static std::shared_ptr <Expression> parseToken(Token&& token, ParserState& state);

private:
	static std::shared_ptr <Operator> parseOneSidedOperator(Token&& token, ParserState& state);
	static std::shared_ptr <Operator> parseTwoSidedOperator(Token&& token, ParserState& state);
};

}

#endif
