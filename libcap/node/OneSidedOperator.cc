#include <cap/node/OneSidedOperator.hh>

namespace cap
{

bool OneSidedOperator::applyCached(std::shared_ptr <Expression>&& cached)
{
	printf("[OneSidedOperator] Apply cached '%s'\n", cached->getToken().c_str());

	if(expression)
	{
		printf("Expression is already set\n");
		return false;
	}

	expression = std::move(cached);
	return true;
}

const char* OneSidedOperator::getTypeString()
{
	switch(type)
	{
		case Type::Negate: return "Negate";
	}

	return "???";
}

unsigned OneSidedOperator::getPrecedence()
{
	// Values from https://en.cppreference.com/w/cpp/language/operator_precedence
	switch(type)
	{
		case Type::Negate: return 3;
	}

	return -1;
}

bool OneSidedOperator::isOneSided()
{
	return true;
}

bool OneSidedOperator::handleLowerPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	printf("[OneSidedOperator] handleLowerPrecedence unimplemented\n");
	return false;
}

bool OneSidedOperator::handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state)
{
	printf("[OneSidedOperator] handleHigherPrecedence unimplemented\n");
	return false;
}

bool OneSidedOperator::handleValue(std::shared_ptr <Value> value, ParserState& state)
{
	printf("[OneSidedOperator] handleValue unimplemented\n");
	return false;
}

}
