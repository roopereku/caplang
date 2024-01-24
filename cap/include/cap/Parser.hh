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

	/// Tells the parser that what was previously parsed is a value.
	void previousWasValue();

	/// Begins an expression and sets the desired root as the current node.
	///
	/// \param root The desired root node for the expression.
	void beginExpression(std::shared_ptr <Expression>&& root);

	/// Begins an expression. Assumes that the current node is the root.
	void beginExpression();

	/// Ends an expression.
	///
	/// \param at The token where the expression enddd.
	/// \return True if succesful.
	bool endExpression(Token& at);

	/// Checks if the parser is currently inside an expression.
	///
	/// \return True if an expression is active.
	bool isInExpression();

	/// Gets the next token excluding comments.
	///
	/// \param tokens The tokens to get the token from.
	/// \result The output token.
	bool getNextToken(Tokenizer& tokens, Token& result);

	/// Parses the given token.
	///
	/// \param token The token to parse.
	/// \param tokens The tokenizer to get more tokens from.
	/// \param breakExpressionOnNewLine If true, an active expression is ended upon a newline.
	bool parseToken(Token& token, Tokenizer& tokens, bool breakExpressionOnNewline);
	
	/// The EventEmitter associated with this parser.
	EventEmitter& events;

private:
	void addNode(std::shared_ptr <Node>&& node);

	bool todo(std::string&& msg);

	bool handleExpressionToken(Token& token);
	bool handleBracketToken(Token& token, Tokenizer& tokens);

	bool parseFunction(Token& token, Tokenizer& tokens);
	bool parseConstructor(Token& token, Tokenizer& tokens);
	bool parseFunctionDefinition(Tokenizer& tokens);

	bool parseType(Token& token, Tokenizer& tokens);
	bool parseInitialization(Token& token);
	bool parseReturn(Token& token);

	bool ensureInitSyntax(std::shared_ptr <Expression> node,
						std::shared_ptr <InitializationRoot> root);

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
