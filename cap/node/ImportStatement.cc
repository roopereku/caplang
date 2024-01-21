#include <cap/node/ImportStatement.hh>

#include <cap/event/ErrorMessage.hh>

namespace cap
{

bool ImportStatement::parse(Token& token, Tokenizer& tokens, Parser& parser)
{
	// Begin an import statement.
	parser.beginExpression(std::make_shared <ImportStatement> (token));
	std::shared_ptr <ImportStatement> importRoot = parser.getCurrentNode()->as <ImportStatement> ();

	// While an expression is active, look for names to import or "from".
	while(parser.isInExpression())
	{
		if(!parser.getNextToken(tokens, token))
		{
			break;
		}

		// If "from" is encountered, start a new expression
		// that specifies where to import from.
		if(token == "from")
		{
			// Try to end the previous expression.
			if(!parser.endExpression(token))
			{
				return false;
			}

			// If "from" was already parsed for this import, throw an error.
			if(importRoot->importFrom)
			{
				parser.events.emit(ErrorMessage("Multiple 'from' in an import", token));
				return false;
			}

			// Initialize the root for "from".
			importRoot->importFrom = std::make_shared <ExpressionRoot> (token);
			importRoot->adopt(importRoot->importFrom);

			// Start an expression for "from".
			parser.setCurrentNode(importRoot->importFrom);
			parser.beginExpression();

			continue;
		}

		if(!parser.parseToken(token, tokens, true))
		{
			return false;
		}
	}

	// Make sure that something is imported.
	if(!importRoot->getRoot())
	{
		parser.events.emit(ErrorMessage("Expected a file or a module after import", token));
		return false;
	}

	return true;
}

std::shared_ptr <ExpressionRoot> ImportStatement::getFrom()
{
	return importFrom;
}

}
