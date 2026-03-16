#ifndef CAP_PARSER_CONTEXT_HH
#define CAP_PARSER_CONTEXT_HH

#include <cap/Token.hh>
#include <cap/Variable.hh>

#include <stack>

namespace cap
{

class Client;
class Source;
class Attribute;
class DeclarationStorage;

class ParserContext
{
public:
	ParserContext(Client& client, Source& source);

	Client& m_client;
	Source& m_source;
};

class Token::ParserContext : public cap::ParserContext
{
public:
	ParserContext(Client& client, Source& source);

	std::stack <Token> m_openedBrackets;
	wchar_t m_previous = 0;
};

class Node::ParserContext : public cap::ParserContext
{
public:
	ParserContext(Client& client, Source& source);

	/// How many nested subexpressions are there currently?
	size_t m_subExpressionDepth = 0;

	/// Used to stop the recursive openings of subexpressions.
	/// Such could happen when a switch to the parent
	/// expression node is made during handleToken.
	bool m_canOpenSubexpression = true;

	/// The declaration storage node to use for new declarations.
	std::shared_ptr <Node> m_declarationLocation;

	/// The node that is exited from as indicated by invokedNodeExited.
	std::shared_ptr <Node> m_exitedFrom;

	/// Is an attribute being parsed?
	bool m_inAttribute = false;

	bool m_allowExpressionEndingInAttributes = false;

	struct ActiveAttributes
	{
		ActiveAttributes(size_t start, size_t depth);

		std::pair <size_t, size_t> m_range;
		size_t m_depth = 0;
	};

	std::stack <ActiveAttributes> m_activeAttributes;

	void storeAttribute(std::shared_ptr <Attribute> attribute);
	void setMoreThanAttributes();
};

}

#endif
