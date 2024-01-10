#include <cap/test/Expect.hh>

namespace cap::test
{

std::shared_ptr <ScopeDefinition> expectScope(std::shared_ptr <Node> node)
{
	if(!node)
	{
		return nullptr;
	}

	return node->type == Node::Type::ScopeDefinition ? node->as <ScopeDefinition> () : nullptr;
}

std::shared_ptr <FunctionDefinition> expectFunction(std::shared_ptr <Node> node)
{
	auto scope = expectScope(node);
	if(!scope)
	{
		return nullptr;
	}

	return scope->type == ScopeDefinition::Type::FunctionDefinition ? node->as <FunctionDefinition> () : nullptr;
}

std::shared_ptr <Expression> expectExpression(std::shared_ptr <Node> node)
{
	if(!node)
	{
		return nullptr;
	}

	return node->type == Node::Type::Expression ? node->as <Expression> () : nullptr;

}

std::shared_ptr <Operator> expectOperator(std::shared_ptr <Node> node)
{
	auto expression = expectExpression(node);
	if(!expression)
	{
		return nullptr;
	}

	return expression->type == Expression::Type::Operator ? node->as <Operator> () : nullptr;
}

std::shared_ptr <OneSidedOperator> expectOneSidedOperator(std::shared_ptr <Node> node)
{
	auto op = expectOperator(node);
	if(!op)
	{
		return nullptr;
	}

	return op->type == Operator::Type::OneSided ? node->as <OneSidedOperator> () : nullptr;

}

std::shared_ptr <TwoSidedOperator> expectTwoSidedOperator(std::shared_ptr <Node> node)
{
	auto op = expectOperator(node);
	if(!op)
	{
		return nullptr;
	}

	return op->type == Operator::Type::TwoSided ? node->as <TwoSidedOperator> () : nullptr;

}

std::shared_ptr <ExpressionRoot> expectExpressionRoot(std::shared_ptr <Node> node)
{
	auto expression = expectExpression(node);
	if(!expression)
	{
		return nullptr;
	}

	return expression->type == Expression::Type::Root ? node->as <ExpressionRoot> () : nullptr;
}

std::shared_ptr <VariableDefinition> expectVariable(std::shared_ptr <Node> node)
{
	auto root = expectExpressionRoot(node);
	if(!root)
	{
		return nullptr;
	}

	return root->type == ExpressionRoot::Type::VariableDefinition ? node->as <VariableDefinition> () : nullptr;
}

std::shared_ptr <ParameterDefinition> expectParameter(std::shared_ptr <Node> node)
{
	auto root = expectExpressionRoot(node);
	if(!root)
	{
		return nullptr;
	}

	return root->type == ExpressionRoot::Type::ParameterDefinition ? node->as <ParameterDefinition> () : nullptr;
}

std::shared_ptr <Value> expectValue(std::shared_ptr <Node> node)
{
	auto expression = expectExpression(node);
	if(!expression)
	{
		return nullptr;
	}

	return expression->type == Expression::Type::Value ? node->as <Value> () : nullptr;

}

std::shared_ptr <CallOperator> expectCall(std::shared_ptr <Node> node)
{
	auto oneSided = expectOneSidedOperator(node);
	if(!oneSided)
	{
		return nullptr;
	}

	return oneSided->type == OneSidedOperator::Type::Call ? node->as <CallOperator> () : nullptr;

}

}
