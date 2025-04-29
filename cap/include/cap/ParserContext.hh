#ifndef CAP_PARSER_CONTEXT_HH
#define CAP_PARSER_CONTEXT_HH

#include <cap/Token.hh>
#include <cap/Variable.hh>

#include <stack>
#include <optional>

namespace cap
{

class Client;
class Source;
class DeclarationStorage;

class ParserContext
{
public:
	ParserContext(Client& client, Source& source)
		: client(client), source(source)
	{
	}

	Client& client;
	Source& source;
};

class Token::ParserContext : public cap::ParserContext
{
public:
	ParserContext(Client& client, Source& source)
		: cap::ParserContext(client, source)
	{
	}

	std::stack <Token> openedBrackets;
	wchar_t previous = 0;
};

class Node::ParserContext : public cap::ParserContext
{
public:
	ParserContext(Client& client, Source& source)
		: cap::ParserContext(client, source)
	{
	}

	/// How many nested subexpressions are there currently?
	size_t subExpressionDepth = 0;

	/// Used to stop the recursive openings of subexpressions.
	/// Such could happen when a switch to the parent
	/// expression node is made during handleToken.
	bool canOpenSubexpression = true;

	/// The declaration storage node to use for new declarations.
	std::shared_ptr <Node> declarationLocation;

	/// The node that is exited from as indicated by invokedNodeExited.
	std::shared_ptr <Node> exitedFrom;
};

}

#endif
