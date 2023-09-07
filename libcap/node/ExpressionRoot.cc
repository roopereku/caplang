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

		if(!first)
		{
			return false;
		}

		if(first->isValue())
		{
			printf("[ExpressionRoot] Cache token '%s'\n", first->getToken().c_str());

			state.cachedValue = std::move(std::static_pointer_cast <Value> (first));
			state.previousIsValue = true;

			return true;
		}

		else
		{
			printf("[ExpressionRoot] Initialize expression\n");

			root = std::move(first);
			state.node = root;
		}
	}

	else
	{
		printf("??? Calling handleToken but root already initialized.\n");
		return false;
	}

	return true;
}

}
