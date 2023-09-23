#include <cap/ParserState.hh>
#include <cap/node/ExpressionRoot.hh>

namespace cap
{

bool ParserState::endExpression()
{
	// If there is a cached value when the expression ends,
	// give it to the most recent expression node.
	if(cachedValue)
	{
		if(!node->isExpression())
		{
			printf("??? Most recent is not expression\n");
			return false;
		}

		auto exprNode = std::static_pointer_cast <Expression> (node);

		if(!exprNode->isExpressionRoot())
		{
			printf("??? Most recent is not expression root\n");
			return false;
		}

		// Give the cached value to the current expression root node.
		std::static_pointer_cast <ExpressionRoot> (exprNode)->setRoot(std::move(cachedValue));
	}

	printf("END EXPRESSION\n");

	node = root->findLastNode();
	inExpression = false;
	previousIsValue = false;
	canEndExpression = false;

	return true;
}

bool ParserState::initExpression(Token::IndexType startRow)
{
	printf("INIT EXPRESSION AT LINE %lu\n", startRow);

	if(inExpression)
	{
		printf("??? initExpression called but expression already active\n");
		return false;
	}

	inExpression = true;
	expressionStartRow = startRow;

	return true;
}

}
