#ifndef CAP_PARSER_HH
#define CAP_PARSER_HH

#include <cap/Node.hh>
#include <cap/Tokenizer.hh>
#include <cap/EventEmitter.hh>

#include <stack>

namespace cap
{

/// Parser parses the given tokens and outputs AST nodes.
class Parser
{
public:
	Parser(Tokenizer& tokens);

	/// Parses the tokens into an AST starting at root.
	///
	/// \param events The EventEmitter to pass events into.
	/// \param root The root of the new AST.
	/// \return True if parsing was succesful.
	bool parse(EventEmitter& events, std::shared_ptr <Node> root);

private:
	void addNode(std::shared_ptr <Node>&& node);

	bool parseNextToken(EventEmitter& events);
	bool todo(std::string&& msg, EventEmitter& events);

	bool handleBracket(Token& token, EventEmitter& events);

	bool parseType(Token& token, EventEmitter& events);
	bool parseFunction(Token& token, EventEmitter& events);
	bool parseVariable(Token& token, EventEmitter& events);

	Tokenizer& tokens;
	std::stack <Token> openingBrackets;

	std::shared_ptr <Node> currentNode;
};

}

#endif
