#include <cap/node/Operator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>

#include <cap/event/DebugMessage.hh>

#include <cassert>

namespace cap
{

bool Operator::handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser)
{
	assert(node);
	parser.events.emit(DebugMessage(std::string("Handle expression in operator ") + getTypeString(), node->token));

	// Adopt the expression node.
	adopt(node);

	// Value nodes should be handled by handleValue().
	assert(node->type != Expression::Type::Value);

	// Is the new node an operator.
	if(node->type == Expression::Type::Operator)
	{
		auto op = node->as <Operator> ();
		bool switchToParent = false;

		parser.events.emit(DebugMessage(std::string("Handle new operator ") + op->getTypeString(), node->token));

		switch(op->type)
		{
			case Type::TwoSided:
			{
				parser.events.emit(DebugMessage("Two sided", node->token));

				// Is the precedence of the new operator higher than the current.
				if(node->as <Operator> ()->getPrecedence() < getPrecedence())
				{
					auto value = stealMostRecentValue();
					op->handleValue(std::move(value));

					parser.setCurrentNode(node);
					handleValue(std::move(node));
				}

				// The precedence is lower or the same.
				else
				{
					switchToParent = true;
				}

				break;
			}

			case Type::OneSided:
			{
				parser.events.emit(DebugMessage("One sided", node->token));

				// Is the precedence of the new operator same as the current.
				if(node->as <Operator> ()->getPrecedence() == getPrecedence())
				{
					switchToParent = true;
				}

				// Precedence is higher or lower, let the current node handle the
				// new operator as a value and set the new operator as the current node.
				else
				{
					// If the operator is complete, the new operator steals the most recent value.
					if(isComplete())
					{
						auto value = stealMostRecentValue();
						op->handleValue(std::move(value));

						parser.events.emit(DebugMessage(std::string("New one sided operator steals the most recent value of ") + getTypeString(), node->token));
					}

					parser.events.emit(DebugMessage(std::string("New one sided operator treated as value by ") + getTypeString(), node->token));

					parser.setCurrentNode(node);
					handleValue(std::move(node));
				}

				break;
			}
		}

		if(switchToParent)
		{
			parser.events.emit(DebugMessage(std::string("Switching to parent of ") + parser.getCurrentNode()->getTypeString(), node->token));
			assert(!getParent().expired());
			parser.setCurrentNode(getParent().lock());

			parser.events.emit(DebugMessage(std::string("Switch to parent ") + parser.getCurrentNode()->getTypeString(), node->token));
			assert(parser.getCurrentNode()->type == Node::Type::Expression);
			parser.getCurrentNode()->as <Expression> ()->handleExpressionNode(node, parser);
		}
	}

	else
	{
		assert(false && "node in Operator::handleExpressionNode() has a weird type");
	}

	return true;
}

}
