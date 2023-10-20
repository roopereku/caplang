#include <cap/ParserState.hh>
#include <cap/node/Operator.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/ExpressionRoot.hh>

#include <cap/event/ErrorMessage.hh>

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

Type& ExpressionRoot::getResultType()
{
	if(!root)
	{
		return Expression::getResultType();
	}

	return root->getResultType();
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
		// If this expression root already has a root, make the new node the root
		// and make the old root an operand of the new root.
		//
		// NOTE: This should only happen if an operator of lower precedence is processed
		// when the current node had higher precedence.
		if(root)
		{
			// TODO: Abstract this somehow.
			if(node->isOperator())
			{
				// If the new node is a two sided operator, adopt the root as lhs.
				if(node->as <Operator> ()->isTwoSided())
				{
					node->as <TwoSidedOperator> ()->setLeft(root);
				}

				// If the new node is a one sided operator, adopt the root as the expression.
				else if(node->as <Operator> ()->isOneSided())
				{
					node->as <OneSidedOperator> ()->setExpression(root);
				}

				node->adopt(root);
			}

			else
			{
				state.events.emit(ErrorMessage(node->getToken(), "??? node isn't an operator"));
				return false;
			}
		}

		// Some operator like FunctionCall require a cached value.
		else if(state.cachedValue)
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
