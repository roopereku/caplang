#ifndef CAP_NODE_EXPRESSION_HH
#define CAP_NODE_EXPRESSION_HH

#include <cap/Node.hh>

namespace cap
{

class Expression : public Node
{
public:
	Expression(Token&& token);

	virtual bool isValue();
	virtual bool isOperator();

	static std::shared_ptr <Expression> parseToken(Token&& token, ParserState& state);
};

}

#endif
