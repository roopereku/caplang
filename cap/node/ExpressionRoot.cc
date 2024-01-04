#include <cap/node/ExpressionRoot.hh>
#include <cap/node/Operator.hh>
#include <cap/Parser.hh>

#include <cap/event/DebugMessage.hh>

#include <cassert>

namespace cap
{

std::shared_ptr <Expression> ExpressionRoot::getRoot()
{
	return root;
}

const char* ExpressionRoot::getTypeString()
{
	return "ExpressionRoot";
}

void ExpressionRoot::setRoot(std::shared_ptr <Expression>&& node, Parser& parser)
{
	parser.events.emit(DebugMessage(std::string("Set ") + node->getTypeString() + " as expression root", node->token));

	root = std::move(node);
	adopt(root);
	parser.setCurrentNode(root);
}

bool ExpressionRoot::handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser)
{
	// If the root node hasn't been initialized, initialize it.
	if(!root)
	{
		setRoot(std::move(node), parser);
	}

	// If there is a root node, let the new node handle the old root
	// and make the new node the new root.
	else
	{
		assert(node->type == Expression::Type::Operator);

		if(!node->as <Operator> ()->handleValue(std::move(root)))
		{
			return false;
		}

		setRoot(std::move(node), parser);
	}

	return true;
}

}
