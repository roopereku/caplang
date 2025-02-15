#include <cap/test/NodeMatcher.hh>

#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/BracketOperator.hh>
#include <cap/Declaration.hh>
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

ExpectedNode::ExpectedNode(cap::BracketOperator::Type type)
	: ExpectedNode(cap::BracketOperator::getTypeString(type))
{
}

ExpectedNode::ExpectedNode(cap::Declaration::Root::Type type)
	: ExpectedNode(cap::Declaration::Root::getTypeString(type))
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

ExpectedNode ClassType(std::wstring&& name)
{
	return ExpectedNode("Class Type", std::move(name));
}

ExpectedNode Expression()
{
	return ExpectedNode("Root");
}

ExpectedNode DeclarationRoot()
{
	return ExpectedNode("Local declaration");
}

ExpectedNode ParameterRoot()
{
	return ExpectedNode("Parameter declaration");
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

	// TODO: Check for Stop?

	if(node->getSignature()->getParameters()->getFirst())
	{
		traverseExpression(node->getSignature()->getParameters()->getFirst());
	}

	auto ret = node->getSignature()->getReturnType();
	if(ret && ret->getFirst())
	{
		traverseExpression(ret->getFirst());
	}

	traverseScope(node->getBody());
	return Traverser::Result::Exit;
}

Traverser::Result NodeMatcher::onClassType(std::shared_ptr <cap::ClassType> node)
{
	auto current = match(node);
	EXPECT_STREQ(node->getName().c_str(), current.context.c_str());

	// TODO: Check for Stop?

	if(node->getGenericRoot())
	{
		traverseExpression(node->getGenericRoot()->getFirst());
	}

	traverseScope(node->getBody());
	return Traverser::Result::Exit;
}

Traverser::Result NodeMatcher::onExpressionRoot(std::shared_ptr <cap::Expression::Root> node)
{
	match(node);
	return Traverser::Result::Continue;
}

Traverser::Result NodeMatcher::onDeclarationRoot(std::shared_ptr <cap::Declaration::Root> node)
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

	return Traverser::Result::Continue;
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
