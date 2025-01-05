#ifndef CAP_TEST_NODE_MATCHER_HH
#define CAP_TEST_NODE_MATCHER_HH

#include <cap/Node.hh>
#include <cap/Traverser.hh>
#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>

#include <vector>

namespace cap::test
{

class ExpectedNode
{
public:
	ExpectedNode(std::string_view nodeType, std::wstring&& context);
	ExpectedNode(std::string_view nodeType);
	ExpectedNode(cap::BinaryOperator::Type type);
	ExpectedNode(cap::BracketOperator::Type type);

	std::string_view nodeType;
	std::wstring context;
};

ExpectedNode Value(std::wstring&& value);
ExpectedNode Scope();
ExpectedNode Function(std::wstring&& name);
ExpectedNode Expression();
ExpectedNode Declaration();

class NodeMatcher : public cap::Traverser
{
public:
	NodeMatcher(std::vector <ExpectedNode>&& expectation);

	Result onScope(std::shared_ptr <cap::Scope> node) override;
	Result onFunction(std::shared_ptr <cap::Function> node) override;
	Result onExpressionRoot(std::shared_ptr <cap::Expression::Root> node) override;
	Result onDeclaration(std::shared_ptr <cap::Declaration> node) override;
	Result onBinaryOperator(std::shared_ptr <cap::BinaryOperator> node) override;
	Result onBracketOperator(std::shared_ptr <cap::BracketOperator> node) override;
	Result onValue(std::shared_ptr <cap::Value> node) override;

	ExpectedNode match(std::shared_ptr <cap::Node> node);

	std::vector <ExpectedNode> expectation;
	size_t current = 0;
};

}

#endif
