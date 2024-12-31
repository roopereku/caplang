#include <cap/test/NodeMatcher.hh>

#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/BracketOperator.hh>
#include <cap/Value.hh>

#include <gtest/gtest.h>

namespace cap::test
{

ExpectedNode::ExpectedNode(std::string_view nodeType, std::wstring&& context)
	: nodeType(nodeType), context(std::move(context))
{
}

ExpectedNode::ExpectedNode(std::string_view nodeType)
	: nodeType(nodeType)
{
}

ExpectedNode::ExpectedNode(cap::BinaryOperator::Type type)
	: ExpectedNode(cap::BinaryOperator::getTypeString(type))
{
}

ExpectedNode Value(std::wstring&& value)
{
	return ExpectedNode("Value", std::move(value));
}

ExpectedNode Scope()
{
	return ExpectedNode("Scope");
}

ExpectedNode Function(std::wstring&& name)
{
	return ExpectedNode("Function", std::move(name));
}

ExpectedNode Expression()
{
	return ExpectedNode("Root");
}

NodeMatcher::NodeMatcher(std::vector <ExpectedNode>&& expectation)
	: expectation(std::move(expectation))
{
}

bool NodeMatcher::onScope(std::shared_ptr <cap::Scope> node)
{
	auto current = match(node);
	return true;
}

bool NodeMatcher::onFunction(std::shared_ptr <cap::Function> node)
{
	auto current = match(node);
	EXPECT_STREQ(node->getName().c_str(), current.context.c_str());
	return true;
}

void NodeMatcher::onExpression(std::shared_ptr <cap::Expression> node)
{
	auto current = match(node);

	// TODO: This could be nicer with more specific traversal functions.
	switch(node->getType())
	{
		case cap::Expression::Type::Root:
		{
			auto root = std::static_pointer_cast <cap::Expression::Root> (node);
			traverseExpression(root->getFirst());

			break;
		}

		case cap::Expression::Type::BinaryOperator:
		{
			auto op = std::static_pointer_cast <cap::BinaryOperator> (node);
			traverseExpression(op->getLeft());
			traverseExpression(op->getRight());

			break;
		}

		case cap::Expression::Type::UnaryOperator:
		{
			break;
		}

		case cap::Expression::Type::BracketOperator:
		{
			auto op = std::static_pointer_cast <cap::BracketOperator> (node);
			traverseExpression(op->getContext());
			traverseExpression(op->getInnerRoot());

			break;

		}

		case cap::Expression::Type::Value:
		{
			auto value = std::static_pointer_cast <cap::Value> (node);
			EXPECT_STREQ(value->getValue().c_str(), current.context.c_str());
			break;
		}
	}
}

ExpectedNode NodeMatcher::match(std::shared_ptr <cap::Node> node)
{
	EXPECT_TRUE(node);
	EXPECT_LT(current, expectation.size());

	auto ret = expectation[current];
	EXPECT_STREQ(node->getTypeString(), ret.nodeType.data());

	current++;
	return ret;
}

}
