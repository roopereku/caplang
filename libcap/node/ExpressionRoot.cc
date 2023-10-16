#include <cap/ParserState.hh>
#include <cap/node/Operator.hh>
#include <cap/node/ExpressionRoot.hh>

namespace cap
{

bool ExpressionRoot::handleToken(Token&& token, ParserState& state)
{
	printf("[ExpressionRoot] Try %s '%s'\n", token.getTypeString(), token.getString().c_str());

	if(!root)
	{
		auto first = parseToken(std::move(token), state);
		if(!handleExpressionNode(first, state))
			return false;	
	}

	else
	{
		printf("??? Calling handleToken but root already initialized.\n");
		return false;
	}

	return true;
}

bool ExpressionRoot::replaceExpression(std::shared_ptr <Expression> node)
{
	root = node;
	adopt(root);

	return true;
}

bool ExpressionRoot::handleExpressionNode(std::shared_ptr <Expression> node, ParserState& state)
{
	adopt(node);

	if(!node)
	{
		return false;
	}

	if(node->isValue())
	{
		//printf("[ExpressionRoot] Cache token '%s'\n", node->getToken().c_str());

		if(state.cachedValue)
		{
			printf("??? There's already a cached value\n");
			return false;
		}

		state.cachedValue = std::move(node->as <Value> ());
		state.previousIsValue = true;

		return true;
	}

	else
	{
		printf("[ExpressionRoot] Initialize expression\n");

		// Some operator like FunctionCall require a cached value.
		if(state.cachedValue)
		{
			if(!node->handleExpressionNode(std::move(state.cachedValue), state))
				return false;
		}

		root = std::move(node);
		adopt(root);

		state.node = root;
	}

	return true;
}

}
