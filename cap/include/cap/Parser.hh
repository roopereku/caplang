#ifndef CAP_PARSER_HH
#define CAP_PARSER_HH

#include <cap/Node.hh>
#include <cap/Tokenizer.hh>
#include <cap/EventEmitter.hh>

#include <cap/node/Expression.hh>
#include <cap/node/InitializationRoot.hh>

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

	/// Sets the current node.
	///
	/// \param node The node to set as current.
	void setCurrentNode(std::shared_ptr <Node> node);

	/// Gets the current node.
	///
	/// \return The current node.
	std::shared_ptr <Node> getCurrentNode();

	/// The EventEmitter associated with this parser.
	EventEmitter& events;

private:
	/// Ignores comments and handles invalid tokens.
	bool getNextToken(Tokenizer& tokens, Token& result);

	void addNode(std::shared_ptr <Node>&& node);

	bool parseToken(Token& token, Tokenizer& tokens, bool breakExpressionOnNewline);
	bool todo(std::string&& msg);

	bool handleExpressionToken(Token& token);
	bool handleBracketToken(Token& token, Tokenizer& tokens);

	bool parseType(Token& token, Tokenizer& tokens);
	bool parseFunction(Token& token, Tokenizer& tokens);
	bool parseInitialization(Token& token, Tokenizer& tokens);

	bool ensureInitSyntax(std::shared_ptr <Expression> node,
						std::shared_ptr <InitializationRoot> root);

	void beginExpression(std::shared_ptr <Expression>&& root);
	bool endExpression(Token& at);

	bool unterminatedBracket(Token at);

	bool isPreviousTokenValue = false;
	Token::IndexType expressionBeginLine = 0;
	bool inExpression = false;

	std::stack <Token> openingBrackets;
	std::shared_ptr <Expression> cachedValue;
	std::shared_ptr <Node> currentNode;
};

}

#endif
