#ifndef CAP_NODE_IMPORT_STATEMENT_HH
#define CAP_NODE_IMPORT_STATEMENT_HH

#include <cap/Parser.hh>

#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class ImportStatement : public ExpressionRoot
{
public:
	ImportStatement(Token& token)
		: ExpressionRoot(Type::ImportStatement, token)
	{
	}

	/// Parses an import statement.
	///
	/// \param token The token containing "import".
	/// \param tokens The tokenizer to get more tokens from.
	/// \param parser The current parser.
	/// \return True if succesful.
	static bool parse(Token& token, Tokenizer& tokens, Parser& parser);

	/// Gets the "from" portion of this import.
	///
	/// \return The expression root containing the "from" portion.
	std::shared_ptr <ExpressionRoot> getFrom();

private:
	std::shared_ptr <ExpressionRoot> importFrom;
};

}

#endif
