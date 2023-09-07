#include <cap/ParserState.hh>

#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/Expression.hh>
#include <cap/node/Value.hh>

namespace cap
{

Expression::Expression(Token&& token) : Node(std::move(token))
{
}

bool Expression::isValue()
{
	return false;
}

bool Expression::isOperator()
{
	return false;
}

std::shared_ptr <Expression> Expression::parseToken(Token&& token, ParserState& state)
{
	if(token.getType() == Token::Type::Operator)
	{
		// If the previous token wasn't a value, assume this to be an unary operator.
		if(!state.previousIsValue)
		{
			printf("TODO: Implement unary operators\n");
			return nullptr;
		}

		// This is not an unary operator.
		else
		{
			state.previousIsValue = false;
			TwoSidedOperator::Type t;

			if(token == "=")
			{
				t = TwoSidedOperator::Type::Assignment;
			}

			else if(token == "+")
			{
				t = TwoSidedOperator::Type::Addition;
			}

			else if(token == "*")
			{
				t = TwoSidedOperator::Type::Multiplication;
			}

			else
			{
				printf("Invalid operator '%s'\n", token.getString().c_str());
				return nullptr;
			}

			auto op = std::make_shared <TwoSidedOperator> (std::move(token), t);

			if(state.cachedValue)
			{
				op->applyCached(std::move(state.cachedValue));
			}

			return op;
		}
	}

	state.previousIsValue = true;

	// The result is a value node.
	return std::make_shared <Value> (std::move(token));
}

}
