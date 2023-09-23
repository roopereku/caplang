#include <cap/ParserState.hh>

namespace cap
{

bool ParserState::endExpression()
{
	printf("END EXPRESSION\n");
	inExpression = false;
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

	node = root->findLastNode();
	inExpression = true;
	expressionStartRow = startRow;

	return true;
}

}
