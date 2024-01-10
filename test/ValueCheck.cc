#include <cap/test/ValueCheck.hh>
#include <cap/test/Expect.hh>

namespace cap::test
{

bool isIdentifier(std::string_view value, std::shared_ptr <Node> node)
{
	return isValue(Token::Type::Identifier, value, node);
}

bool isInteger(std::string_view value, std::shared_ptr <Node> node)
{
	return isValue(Token::Type::Integer, value, node);
}

bool isValue(Token::Type type, std::string_view value, std::shared_ptr <Node> node)
{
	auto valueNode = expectValue(node);
	if(!valueNode)
	{
		return false;
	}

	return valueNode->token.getType() == type && valueNode->token == value;
}

}
