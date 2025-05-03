#include <cap/test/NodeMatcher.hh>

#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/BracketOperator.hh>
#include <cap/Variable.hh>
#include <cap/Value.hh>
#include <cap/Return.hh>

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

ExpectedNode::ExpectedNode(cap::BracketOperator::Type type)
	: ExpectedNode(cap::BracketOperator::getTypeString(type))
{
}

ExpectedNode::ExpectedNode(cap::Variable::Type type)
	: ExpectedNode(cap::Variable::getTypeString(type))
{
}

ExpectedNode::ExpectedNode(cap::ModifierRoot::Type type)
	: ExpectedNode(cap::ModifierRoot::getTypeString(type))
{
}

ExpectedNode Value(std::wstring&& value, std::wstring&& referred)
{
	auto expected = ExpectedNode("Value", std::move(value));
	expected.referred = std::move(referred);
	
	return expected;
}

ExpectedNode Scope()
{
	return ExpectedNode("Scope");
}

ExpectedNode Function(std::wstring&& name)
{
	return ExpectedNode("Function", std::move(name));
}

ExpectedNode ClassType(std::wstring&& name)
{
	return ExpectedNode("Class Type", std::move(name));
}

ExpectedNode Expression()
{
	return ExpectedNode("Expression root");
}

ExpectedNode LocalVariable(std::wstring&& name)
{
	return ExpectedNode("Local variable", std::move(name));
}

ExpectedNode Parameter(std::wstring&& name)
{
	return ExpectedNode("Parameter", std::move(name));
}

ExpectedNode Generic(std::wstring&& name)
{
	return ExpectedNode("Generic", std::move(name));
}

ExpectedNode Return()
{
	return ExpectedNode("Return");
}

NodeMatcher::NodeMatcher(std::vector <ExpectedNode>&& expectation)
	: expectation(std::move(expectation))
{
}

Traverser::Result NodeMatcher::onScope(std::shared_ptr <cap::Scope> node)
{
	auto current = match(node);
	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onFunction(std::shared_ptr <cap::Function> node)
{
	auto current = match(node);
	EXPECT_STREQ(node->getName().c_str(), current.context.c_str());
	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onClassType(std::shared_ptr <cap::ClassType> node)
{
	auto current = match(node);
	EXPECT_STREQ(node->getName().c_str(), current.context.c_str());
	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onExpressionRoot(std::shared_ptr <cap::Expression::Root> node)
{
	match(node);
	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onVariable(std::shared_ptr <cap::Variable> node)
{
	match(node);
	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onModifierRoot(std::shared_ptr <cap::ModifierRoot> node)
{
	match(node);
	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onBinaryOperator(std::shared_ptr <cap::BinaryOperator> node)
{
	match(node);

	EXPECT_TRUE(node->getLeft());
	EXPECT_TRUE(node->getRight());

	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onBracketOperator(std::shared_ptr <cap::BracketOperator> node)
{
	match(node);

	EXPECT_TRUE(node->getContext());
	EXPECT_TRUE(node->getInnerRoot());

	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onValue(std::shared_ptr <cap::Value> node)
{
	auto current = match(node);
	EXPECT_STREQ(node->getValue().c_str(), current.context.c_str());

	if(!current.referred.empty())
	{
		EXPECT_TRUE(node->getReferred() != nullptr);
		EXPECT_STREQ(node->getReferred()->getLocation().c_str(), current.referred.c_str());
	}

	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onReturn(std::shared_ptr <cap::Return> node)
{
	match(node);
	return Traverser::Result::Continue;
}

ExpectedNode NodeMatcher::match(std::shared_ptr <cap::Node> node)
{
	EXPECT_TRUE(node);
	EXPECT_LT(current, expectation.size());

	auto ret = expectation[current];
	EXPECT_STREQ(node->getTypeString(), ret.nodeType.data());

	if(node->getType() == cap::Node::Type::Expression && !ret.resultType.empty())
	{
		auto expr = std::static_pointer_cast <cap::Expression> (node);
		EXPECT_STREQ(expr->getResultType().toString().c_str(), ret.resultType.c_str());
	}

	current++;
	return ret;
}

}
