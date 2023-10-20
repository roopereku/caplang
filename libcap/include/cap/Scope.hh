#ifndef CAP_SCOPE_HH
#define CAP_SCOPE_HH

#include <cap/EventEmitter.hh>
#include <cap/ValidationState.hh>
#include <cap/ParserState.hh>
#include <cap/Node.hh>
#include <cap/node/Declaration.hh>

#include <memory>
#include <vector>

namespace cap
{

class Type;
class NamedScope;

class Scope
{
public:
	Scope(Scope& parent) : Scope(&parent)
	{
	}

	Scope(const Scope& rhs) = delete;

	/// Gets the shared scope which is a scope that all scopes should reside in.
	///
	/// \returns Reference to the shared scope.
	static Scope& getSharedScope();

	/// Gets the declaration of the given member if it exists.
	///
	/// \param name The name of the member declaration.
	/// \returns Declaration of the given member if it exists.
	std::shared_ptr <Declaration> getMember(std::string_view name);

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

	/// Creates a new primitive type.
	///
	/// \param name The name of the new type.
	/// \param size How many bytes does this primitive take.
	/// \returns The newly created primitive type.
	Type& createPrimitiveType(std::string_view name, size_t size);

	/// Creates a new variable from the next tokens.
	///
	/// \param token The token containing "var".
	/// \param state The state of the parser.
	/// \param isParameter Determines whether following variables are parameters.
	/// \returns True if the variable was created succesfully.
	bool createVariable(Token&& token, ParserState& state, bool isParameter);

	/// Parses a scope from the given tokens.
	///
	/// \param tokens The tokens to parse a scope from.
	/// \param events EventEmitter to send events to.
	/// \returns True if parsing was succesful.
	bool parse(Tokenizer& tokens, EventEmitter& events);

	/// Parses a scope using the given state.
	///
	/// \param tokens The state the use during parsing.
	/// \returns True if parsing was succesful.
	virtual bool parse(ParserState& state);

	/// Validates this scope.
	///
	/// \param events EventEmitter to push events to.
	/// \returns True if validation was succesful.
	bool validate(EventEmitter& events);

	const std::shared_ptr <Node> getRoot() const
	{
		return root;
	}

	virtual bool isNamed()
	{
		return false;
	}

private:
	Scope();

	Scope(Scope* parent) : parent(parent),
		root(std::make_shared <Node> (Token::createInvalid()))
	{
		printf("Create scope\n");
	}

	bool validateNode(std::shared_ptr <Node> node, ValidationState& state);
	std::shared_ptr <Expression> validateExpression(std::shared_ptr <Expression> expr, ValidationState& state);

	bool handleVariableDeclaration(std::shared_ptr <Expression> node, ValidationState& state);

	bool parseBracket(Token&& token, ParserState& state);
	bool checkRowChange(Token::IndexType currentRow, ParserState& state);

	static Token consumeName(Tokenizer& tokens);

	Scope* parent;
	std::vector <std::shared_ptr <Declaration>> members;
	std::shared_ptr <Node> root;
};

}

#endif
