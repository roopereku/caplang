#ifndef CAP_PARSER_HH
#define CAP_PARSER_HH

#include <cap/Node.hh>
#include <cap/Tokenizer.hh>
#include <cap/EventEmitter.hh>

#include <cap/node/Expression.hh>

#include <stack>

namespace cap
{

/// Parser parses the given tokens and outputs AST nodes.
class Parser
{
public:
	Parser(EventEmitter& events);

	/// Parses the tokens into an AST starting at root.
	///
	/// \param tokens The tokens to parse an AST from.
	/// \param root The root of the new AST.
	/// \return True if parsing was succesful.
	bool parse(Tokenizer& tokens, std::shared_ptr <Node> root);

	void setCurrentNode(std::shared_ptr <Node> node);
	std::shared_ptr <Node> getCurrentNode();

	EventEmitter& events;

private:
	void addNode(std::shared_ptr <Node>&& node);

	bool parseNextToken(Tokenizer& tokens);
	bool todo(std::string&& msg);

	bool handleExpressionToken(Token& token);
	bool handleBracketToken(Token& token);

	bool parseType(Token& token, Tokenizer& tokens);
	bool parseFunction(Token& token, Tokenizer& tokens);
	bool parseVariable(Token& token, Tokenizer& tokens);

	void beginExpression(Token& at);
	void endExpression(Token& at);

	bool isPreviousTokenValue = false;
	unsigned expressionBraceDepth = 0;
	size_t expressionBeginLine = 0;
	bool inExpression = false;

	std::stack <Token> openingBrackets;
	std::shared_ptr <Expression> cachedValue;
	std::shared_ptr <Node> currentNode;
};

}

#endif
