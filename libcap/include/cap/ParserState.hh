#ifndef CAP_PARSER_STATE_HH
#define CAP_PARSER_STATE_HH

#include <cap/Tokenizer.hh>
#include <cap/EventEmitter.hh>
#include <cap/Node.hh>
#include <cap/BraceMatcher.hh>
#include <cap/node/Expression.hh>

namespace cap
{

struct ParserState
{
	ParserState(Tokenizer& tokens, EventEmitter& events, std::shared_ptr <Node> root)
		: tokens(tokens), events(events), root(std::move(root)), node(this->root)
	{
	}

	bool endExpression();
	bool initExpression(Token::IndexType startRow);

	BraceMatcher braces;
	Tokenizer& tokens;
	EventEmitter& events;

	bool inExpression = false;
	bool previousIsValue = false;
	bool canEndExpression = true;
	Token::IndexType expressionStartRow;

	std::shared_ptr <Expression> cachedValue;
	std::shared_ptr <Node> root;
	std::shared_ptr <Node> node;
};

}

#endif
