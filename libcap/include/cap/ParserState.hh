#ifndef CAP_PARSER_STATE_HH
#define CAP_PARSER_STATE_HH

#include <cap/Tokenizer.hh>
#include <cap/Node.hh>
#include <cap/BraceMatcher.hh>
#include <cap/node/Expression.hh>

namespace cap
{

struct ParserState
{
	ParserState(Tokenizer& tokens, std::shared_ptr <Node> node)
		: tokens(tokens), node(std::move(node))
	{
	}

	BraceMatcher braces;
	Tokenizer& tokens;

	bool inExpression = false;
	bool previousIsValue = false;
	bool canEndExpression = true;
	Token::IndexType expressionStartRow;

	std::shared_ptr <Expression> cachedValue;
	std::shared_ptr <Node> node;
};

}

#endif
