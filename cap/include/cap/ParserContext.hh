#ifndef CAP_PARSER_CONTEXT_HH
#define CAP_PARSER_CONTEXT_HH

#include <cap/Token.hh>
#include <cap/Declaration.hh>

#include <stack>

namespace cap
{

class Client;
class Source;

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

	/// Used to insert a "let" at the beginning of the next expression.
	/// This is skipped if a closing bracket comes first.
	Declaration::Root::Type implicitDeclaration = Declaration::Root::Type::None;
};

}

#endif
