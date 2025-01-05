#include <cap/Traverser.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/Expression.hh>
#include <cap/Declaration.hh>
#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>
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
	Result result;

	switch(node->getType())
	{
		case Scope::Type::Standalone:
		{
			result = onScope(node);
			break;
		}

		case Scope::Type::Function:
		{
			result = onFunction(std::static_pointer_cast <Function> (node));

			// TODO: Traverse to the return value and signature?
			if(shouldContinue(result))
			{
			}

			break;
		}

		case Scope::Type::ClassType:
		{
			result = onClassType(std::static_pointer_cast <ClassType> (node));

			// TODO: Traverse to the base classes?
			if(shouldContinue(result))
			{
			}

			break;
		}

		case Scope::Type::Custom:
		{
			result = onCustomScope(node);
			break;
		}
	}

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
	return true;
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

		case Expression::Type::Declaration:
		{
			auto decl = std::static_pointer_cast <Declaration> (node);
			result = onDeclaration(decl);

			if(shouldContinue(result))
			{
				if(!traverseExpression(decl->getFirst()))
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

		case Expression::Type::UnaryOperator: assert(false && "UnaryOperator Unimplemented");
	}

	onNodeExited(node, result);
	return true;
}

void Traverser::onNodeExited(std::shared_ptr <Node>, Result) {}
Traverser::Result Traverser::onCustomNode(std::shared_ptr <Node>) { return Result::NotHandled; }
Traverser::Result Traverser::onScope(std::shared_ptr <Scope>) { return Result::NotHandled; }
Traverser::Result Traverser::onFunction(std::shared_ptr <Function>) { return Result::NotHandled; }
Traverser::Result Traverser::onClassType(std::shared_ptr <ClassType>) { return Result::NotHandled; }
Traverser::Result Traverser::onCustomScope(std::shared_ptr <Scope>) { return Result::NotHandled; }
Traverser::Result Traverser::onExpressionRoot(std::shared_ptr <Expression::Root>) {return Result::NotHandled; }
Traverser::Result Traverser::onDeclaration(std::shared_ptr <Declaration>) {return Result::NotHandled; }
Traverser::Result Traverser::onBinaryOperator(std::shared_ptr <BinaryOperator>) {return Result::NotHandled; }
Traverser::Result Traverser::onBracketOperator(std::shared_ptr <BracketOperator>) {return Result::NotHandled; }
Traverser::Result Traverser::onValue(std::shared_ptr <Value>) {return Result::NotHandled; }

}
