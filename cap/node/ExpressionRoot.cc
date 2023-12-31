#include <cap/node/ExpressionRoot.hh>
#include <cap/Parser.hh>

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

bool ExpressionRoot::handleExpressionNode(std::shared_ptr <Expression> node, Parser& parser)
{
	if(!root)
	{
		root = std::move(node);
		adopt(root);

		parser.setCurrentNode(root);
	}

	else
	{
		root->handleExpressionNode(node, parser);
	}

	return true;
}

bool ExpressionRoot::replaceExpression(std::shared_ptr <Expression> node)
{
	root = std::move(node);
	adopt(root);

	return true;
}

}
