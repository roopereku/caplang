#include <cap/ParserState.hh>

#include <cap/node/OneSidedOperator.hh>
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

bool Expression::isVariableDeclaration()
{
	return false;
}

bool Expression::isExpressionRoot()
{
	return false;
}

bool Expression::replaceExpression(std::shared_ptr <Expression> node)
{
	printf("called Expression::replaceExpression\n");
	return false;
}

bool Expression::handleExpressionNode(std::shared_ptr <Expression> node, ParserState& state)
{
	printf("called Expression::handleExpressionNode\n");
	return false;
}

std::shared_ptr <Expression> Expression::parseToken(Token&& token, ParserState& state)
{
	if(token.getType() == Token::Type::Operator)
	{
		std::shared_ptr <Operator> op;

		// If the previous token wasn't a value, assume this to be an unary operator.
		if(!state.previousIsValue)
		{
			printf("Parse one sided\n");
			op = parseOneSidedOperator(std::move(token), state);
		}

		// This is not an unary operator.
		else
		{
			printf("Parse two sided\n");
			op = parseTwoSidedOperator(std::move(token), state);
		}

		if(!op)
		{
			printf("Invalid operator '%s'\n", token.getString().c_str());
			return nullptr;
		}

		if(state.cachedValue)
		{
			op->applyCached(std::move(state.cachedValue));
		}

		state.previousIsValue = false;

		return op;
	}

	// The result is a value node.
	state.previousIsValue = true;
	return std::make_shared <Value> (std::move(token));
}

std::shared_ptr <Operator> Expression::parseOneSidedOperator(Token&& token, ParserState& state)
{
	OneSidedOperator::Type t;

	if(token == "-")
	{
		t = OneSidedOperator::Type::Negate;
	}

	else
	{
		return nullptr;
	}

	return std::make_shared <OneSidedOperator> (std::move(token), t);
}

std::shared_ptr <Operator> Expression::parseTwoSidedOperator(Token&& token, ParserState& state)
{
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

	else if(token == "/")
	{
		t = TwoSidedOperator::Type::Division;
	}

	else if(token == ".")
	{
		t = TwoSidedOperator::Type::Access;
	}

	else
	{
		return nullptr;
	}

	return std::make_shared <TwoSidedOperator> (std::move(token), t);
}

}
