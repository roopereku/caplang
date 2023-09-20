#ifndef CAP_SCOPE_HH
#define CAP_SCOPE_HH

#include <cap/ParserState.hh>
#include <cap/Variable.hh>
#include <cap/Node.hh>

#include <memory>
#include <vector>

namespace cap
{

class Scope
{
public:
	Scope(Scope& parent) : Scope(&parent)
	{
	}

	Scope() : Scope(nullptr)
	{
	}

	/// Creates a new function from the next tokens.
	///
	/// \param token The token containing "func".
	/// \param state The state of the parser.
	/// \returns True if the function was created succesfully.
	bool createFunction(Token&& token, ParserState& state);

	/// Creates a new type from the next tokens.
	///
	/// \param token The token containing "type".
	/// \param state The state of the parser.
	/// \returns True if the type was created succesfully.
	bool createType(Token&& token, ParserState& state);

	/// Creates a new variable from the next tokens.
	///
	/// \param state The state of the parser.
	/// \returns True if the variable was created succesfully.
	bool createVariable(ParserState& state);

	bool parse(Tokenizer& tokens);
	virtual bool parse(ParserState& state);

	const std::shared_ptr <Node> getRoot() const
	{
		return root;
	}

private:
	Scope(Scope* parent) : parent(parent),
		root(std::make_shared <Node> (Token::createInvalid()))
	{
		printf("Create scope\n");
	}

	bool parseBracket(Token&& token, ParserState& state);

	std::shared_ptr <Node> findLastNode();

	static Token consumeName(Tokenizer& tokens);

	std::vector <std::shared_ptr <Scope>> scopes;
	Scope* parent;

	std::shared_ptr <Node> root;
};

}

#endif
