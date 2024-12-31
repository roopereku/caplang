#ifndef CAP_TEST_NODE_MATCHER_HH
#define CAP_TEST_NODE_MATCHER_HH

#include <cap/Node.hh>
#include <cap/BinaryOperator.hh>

#include <vector>

namespace cap::test
{

class ExpectedNode
{
public:
	ExpectedNode(std::string_view nodeType, std::wstring&& context);
	ExpectedNode(std::string_view nodeType);
	ExpectedNode(cap::BinaryOperator::Type type);

	std::string_view nodeType;
	std::wstring context;
};

ExpectedNode Value(std::wstring&& value);
ExpectedNode Scope();
ExpectedNode Function(std::wstring&& name);
ExpectedNode Expression();

class NodeMatcher : public cap::Node::Traverser
{
public:
	NodeMatcher(std::vector <ExpectedNode>&& expectation);

	bool onScope(std::shared_ptr <cap::Scope> node) override;
	bool onFunction(std::shared_ptr <cap::Function> node) override;
	void onExpression(std::shared_ptr <cap::Expression> node) override;
	ExpectedNode match(std::shared_ptr <cap::Node> node);

	std::vector <ExpectedNode> expectation;
	size_t current = 0;
};

}

#endif
