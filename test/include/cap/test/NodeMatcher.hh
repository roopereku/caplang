#ifndef CAP_TEST_NODE_MATCHER_HH
#define CAP_TEST_NODE_MATCHER_HH

#include <cap/Node.hh>
#include <cap/Traverser.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Variable.hh>

#include <vector>
#include <cstdint>

namespace cap::test
{

class ExpectedNode
{
public:
	ExpectedNode(std::string_view nodeType, std::wstring&& context);
	ExpectedNode(std::string_view nodeType);
	ExpectedNode(cap::BinaryOperator::Type type);
	ExpectedNode(cap::UnaryOperator::Type type);
	ExpectedNode(cap::BracketOperator::Type type);
	ExpectedNode(cap::Variable::Type type);

	std::string_view nodeType;
	std::wstring resultType;
	std::wstring referred;
	std::wstring context;
};

ExpectedNode Identifier(std::wstring&& value, std::wstring&& referred = L"");
ExpectedNode Integer(uint64_t value);
ExpectedNode String(std::wstring&& value);
ExpectedNode Scope();
ExpectedNode Function(std::wstring&& name);
ExpectedNode ClassType(std::wstring&& name);
ExpectedNode Expression();
ExpectedNode LocalVariable(std::wstring&& name);
ExpectedNode Parameter(std::wstring&& name);
ExpectedNode Generic(std::wstring&& name);
ExpectedNode TypeReference();
ExpectedNode Return();

class NodeMatcher : public cap::Traverser
{
public:
	NodeMatcher(std::vector <ExpectedNode>&& expectation);

	Result onScope(std::shared_ptr <cap::Scope> node) override;
	Result onFunction(std::shared_ptr <cap::Function> node) override;
	Result onClassType(std::shared_ptr <cap::ClassType> node) override;
	Result onExpressionRoot(std::shared_ptr <cap::Expression::Root> node) override;
	Result onVariable(std::shared_ptr <cap::Variable> node) override;
	Result onTypeReference(std::shared_ptr <cap::TypeReference> node) override;
	Result onBinaryOperator(std::shared_ptr <cap::BinaryOperator> node) override;
	Result onUnaryOperator(std::shared_ptr <cap::UnaryOperator> node) override;
	Result onBracketOperator(std::shared_ptr <cap::BracketOperator> node) override;
	Result onIdentifier(std::shared_ptr <cap::Identifier> node) override;
	Result onInteger(std::shared_ptr <cap::Integer> node) override;
	Result onString(std::shared_ptr <cap::String> node) override;
	Result onReturn(std::shared_ptr <cap::Return> node) override;

	ExpectedNode match(std::shared_ptr <cap::Node> node);

	std::vector <ExpectedNode> expectation;
	size_t current = 0;
};

}

template <typename T>
cap::test::ExpectedNode operator>(T&& expected, std::wstring_view resultType)
{
	cap::test::ExpectedNode result(std::forward <T> (expected));
	result.resultType = std::wstring(resultType);
	return result;
}

#endif
