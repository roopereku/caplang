#ifndef CAP_TEST_VALUE_CHECK_HH
#define CAP_TEST_VALUE_CHECK_HH

#include <cap/node/Value.hh>

namespace cap::test
{

bool isIdentifier(std::string_view value, std::shared_ptr <Node> node);
bool isInteger(std::string_view value, std::shared_ptr <Node> node);
bool isValue(Token::Type type, std::string_view value, std::shared_ptr <Node> node);

}

#endif
