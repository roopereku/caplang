#include <cap/Traverser.hh>
#include <cap/ArgumentAccessor.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/CallableType.hh>
#include <cap/Expression.hh>
#include <cap/Declaration.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/TypeDefinition.hh>
#include <cap/ModifierRoot.hh>
#include <cap/Variable.hh>
#include <cap/Return.hh>
#include <cap/Value.hh>

#include <cassert>

namespace cap
{

constexpr bool shouldContinue(Traverser::Result result)
{
	return result == Traverser::Result::Continue || result == Traverser::Result::NotHandled;
}

bool Traverser::traverseNode(std::shared_ptr <Node> node)
{
	switch(node->getType())
	{
		case Node::Type::Scope: return traverseScope(std::static_pointer_cast <Scope> (node));
		case Node::Type::Expression: return traverseExpression(std::static_pointer_cast <Expression> (node));
		case Node::Type::Declaration: return traverseDeclaration(std::static_pointer_cast <Declaration> (node));
		case Node::Type::Statement: return traverseStatement(std::static_pointer_cast <Statement> (node));

		case Node::Type::Custom:
		{
			auto result = onCustomNode(node);
			onNodeExited(node, result);
			return result != Result::Stop;
		}
	}

	assert(false && "Invalid node type");
	return false;
}

bool Traverser::traverseScope(std::shared_ptr <Scope> node)
{
	Result result = onScope(node);

	if(shouldContinue(result))
	{
		for(auto nested : node->getNested())
		{
			if(!traverseNode(nested))
			{
				onNodeExited(node, result);
				return false;
			}
		}
	}

	onNodeExited(node, result);
	return result != Result::Stop;
}

bool Traverser::traverseExpression(std::shared_ptr <Expression> node)
{
	Result result;

	switch(node->getType())
	{
		case Expression::Type::Root:
		{
			auto root = std::static_pointer_cast <Expression::Root> (node);
			result = onExpressionRoot(root);

			if(shouldContinue(result) && root->getFirst())
			{
				if(!traverseExpression(root->getFirst()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}

		case Expression::Type::Value:
		{
			result = onValue(std::static_pointer_cast <Value> (node));
			break;
		}

		case Expression::Type::ModifierRoot:
		{
			auto modifier = std::static_pointer_cast <ModifierRoot> (node);
			result = onModifierRoot(modifier);

			if(shouldContinue(result) && modifier->getFirst())
			{
				if(!traverseExpression(modifier->getFirst()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}

		case Expression::Type::BinaryOperator:
		{
			auto op = std::static_pointer_cast <BinaryOperator> (node);
			result = onBinaryOperator(op);

			if(shouldContinue(result))
			{
				if(!traverseExpression(op->getLeft()) ||
					!traverseExpression(op->getRight()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}

		case Expression::Type::UnaryOperator:
		{
			auto op = std::static_pointer_cast <UnaryOperator> (node);
			result = onUnaryOperator(op);

			if(shouldContinue(result))
			{
				if(!traverseExpression(op->getExpression()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}

		case Expression::Type::BracketOperator:
		{
			auto op = std::static_pointer_cast <BracketOperator> (node);
			result = onBracketOperator(op);

			if(shouldContinue(result))
			{
				if(!traverseExpression(op->getContext()) ||
					!traverseExpression(op->getInnerRoot()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}
	}

	onNodeExited(node, result);
	return result != Result::Stop;
}

bool Traverser::traverseDeclaration(std::shared_ptr <Declaration> node)
{
	Result result;

	switch(node->getType())
	{
		case Declaration::Type::TypeDefinition:
		{
			return traverseTypeDefinition(std::static_pointer_cast <TypeDefinition> (node));
		}

		case Declaration::Type::Function:
		{
			auto function = std::static_pointer_cast <Function> (node);
			result = onFunction(function);

			if(shouldContinue(result))
			{
				if(!traverseDeclaration(function->getSignature()) ||
					!traverseScope(function->getBody()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}

		case Declaration::Type::Variable:
		{
			auto variable = std::static_pointer_cast <Variable> (node);
			result = onVariable(variable);

			if(shouldContinue(result))
			{
				if(!traverseExpression(variable->getInitialization()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}
	}

	onNodeExited(node, result);
	return result != Result::Stop;
}

bool Traverser::traverseTypeDefinition(std::shared_ptr <TypeDefinition> node)
{
	Result result;

	switch(node->getType())
	{
		case TypeDefinition::Type::Class:
		{
			auto classType = std::static_pointer_cast <ClassType> (node);
			result = onClassType(classType);

			// TODO: Traverse to the base classes?
			if(shouldContinue(result))
			{
				// If there's no generic root, report success to skip it.
				bool genericTraverseResult = !classType->getGenericRoot() ||
					traverseStatement(classType->getGenericRoot());

				if(!genericTraverseResult ||
					!traverseScope(classType->getBody()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			break;
		}

		case TypeDefinition::Type::Callable:
		{
			auto callable = std::static_pointer_cast <CallableType> (node);
			result = onCallableType(callable);

			if(shouldContinue(result))
			{
				auto ret = callable->getReturnTypeRoot();
				auto param = callable->getParameterRoot();

				if((param && !traverseStatement(param)) ||
					(ret && !traverseExpression(ret)))
				{
					onNodeExited(node, result);
					return false;
				}
			}
		}
	}

	onNodeExited(node, result);
	return result != Result::Stop;
}

bool Traverser::traverseStatement(std::shared_ptr <Statement> node)
{
	Result result;

	switch(node->getType())
	{
		case Statement::Type::VariableRoot:
		{
			auto variableRoot = std::static_pointer_cast <Variable::Root> (node);
			ArgumentAccessor args(variableRoot);
			result = Result::Exit;

			// Instead of traversing through the expression nodes within the variable
			// root, just traverse through the variable declarations to exclude
			// commas and assignments. This makes the AST a bit clearer.
			while(auto expr = args.getNext())
			{
				assert(expr->getType() == Expression::Type::BinaryOperator);
				auto op = std::static_pointer_cast <BinaryOperator> (expr);

				assert(op->getLeft()->getType() == Expression::Type::Value);
				auto name = std::static_pointer_cast <Value> (op->getLeft());

				assert(name->getReferred());
				if(!traverseDeclaration(name->getReferred()))
				{
					onNodeExited(node, result);
					return false;
				}
			}

			// Stop early to prevent onNodeExited being fired for Variable::Root.
			return result != Result::Stop;
		}

		case Statement::Type::Return:
		{
			auto ret = std::static_pointer_cast <Return> (node);
			result = onReturn(ret);

			if(shouldContinue(result))
			{
				if(!traverseExpression(ret->getExpression()))
				{
					onNodeExited(node, result);
					return false;
				}
			}
		}
	}

	onNodeExited(node, result);
	return result != Result::Stop;
}

void Traverser::onNodeExited(std::shared_ptr <Node>, Result) {}
Traverser::Result Traverser::onCustomNode(std::shared_ptr <Node>) { return Result::NotHandled; }
Traverser::Result Traverser::onScope(std::shared_ptr <Scope>) { return Result::NotHandled; }
Traverser::Result Traverser::onFunction(std::shared_ptr <Function>) { return Result::NotHandled; }
Traverser::Result Traverser::onClassType(std::shared_ptr <ClassType>) { return Result::NotHandled; }
Traverser::Result Traverser::onCallableType(std::shared_ptr <CallableType>) { return Result::NotHandled; }
Traverser::Result Traverser::onExpressionRoot(std::shared_ptr <Expression::Root>) { return Result::NotHandled; }
Traverser::Result Traverser::onModifierRoot(std::shared_ptr <ModifierRoot>) { return Result::NotHandled; }
Traverser::Result Traverser::onBinaryOperator(std::shared_ptr <BinaryOperator>) { return Result::NotHandled; }
Traverser::Result Traverser::onUnaryOperator(std::shared_ptr <UnaryOperator>) { return Result::NotHandled; }
Traverser::Result Traverser::onBracketOperator(std::shared_ptr <BracketOperator>) { return Result::NotHandled; }
Traverser::Result Traverser::onValue(std::shared_ptr <Value>) { return Result::NotHandled; }
Traverser::Result Traverser::onVariable(std::shared_ptr <Variable>) { return Result::NotHandled; }
Traverser::Result Traverser::onReturn(std::shared_ptr <Return>) { return Result::NotHandled; }

}
