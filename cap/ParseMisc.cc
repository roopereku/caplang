#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"
#include <memory>

bool Cap::SourceFile::parseMisc(size_t& i, Scope& current)
{
	if(tokens[i].stringEquals("return"))
	{
		current.node->type = SyntaxTreeNode::Type::Return;
		current.node->value = &tokens[i];

		//	Create a node for the expression coming after the return
		SyntaxTreeNode* old = current.node;
		current.node->left = std::make_shared <SyntaxTreeNode> (current.node);
		current.node = current.node->left.get();

		//	Parse an expression after the return
		i++;
		bool result = parseLine(i, current);

		//	If an expression wasn't present, throw an error
		if(old->left->type == SyntaxTreeNode::Type::None)
		{
			Logger::error(*old->value, "Expected an expression after 'return'");
			return errorOut();
		}

		current.node = old;
		return result;
	}

	SyntaxTreeNode::Type which = SyntaxTreeNode::Type::None;

	//	First check if the token equals to something that can't be used inside an expression
	if(tokens[i].stringEquals("if")) which = SyntaxTreeNode::Type::If;
	else if(tokens[i].stringEquals("while")) which = SyntaxTreeNode::Type::While;

	if(which == SyntaxTreeNode::Type::None)
		return false;

	if(inExpression)
	{
		Logger::error(tokens[i], "Cannot use %s inside an expression", tokens[i].getString().c_str());
		return errorOut();
	}

	//	TODO context could be something specific such as "when"
	Scope& scope = current.addBlock(ScopeContext::Block);
	Token* name = &tokens[i];

	i++;
	if(!isToken(TokenType::Parenthesis, i))
	{
		Logger::error(tokens[i], "Expected parenthesis after %s", name->getString().c_str());
		return errorOut();
	}

	i++;
	size_t parenthesisStart = i;

	scope.root.type = which;

	//	Create a root node for the expression inside the parenthesis
	scope.root.left = std::make_shared <SyntaxTreeNode> (&scope.root);
	scope.node = scope.root.left.get();

	//	Parse the contents of the parenthesis
	if(!parseLine(i, scope, true))
		return errorOut();

	//	If there is node left node, or the left node contains nothing, the parentheses are empty
	if(!scope.root.left || scope.root.left->type == SyntaxTreeNode::Type::None)
	{
		//	FIXME Depending on the context, emit a different more fitting error message
		Logger::error(tokens[parenthesisStart], "The parenthesis of '%s' are empty", name->getString().c_str());
		return errorOut();
	}

	//	Create a root node for the for the contents of the block
	scope.root.right = std::make_shared <SyntaxTreeNode> (&scope.root);
	scope.node = scope.root.right.get();

	i++;
	if(!parseBody(i, scope))
		return errorOut();

	//	If the miscellaneous block is valid, update the current node to represent it
	current.node->type = SyntaxTreeNode::Type::Block;

	//	Though this is a little cryptic, let's store the scope index to the token length to save space
	current.node->value = name;
	current.node->value->length = current.getBlockCount() - 1;

	DBG_LOG("Scope number %u", current.node->value->length);

	//	Create a new node on the right to represent a new line
	current.node->right = std::make_shared <SyntaxTreeNode> (current.node);
	current.node = current.node->right.get();

	return true;
}

bool Cap::SourceFile::parseBody(size_t& i, Scope& current)
{
	bool result;

	//	Does the function have a body inside curly braces?
	if(isToken(TokenType::CurlyBrace, i) && tokens[i].length > 0)
	{
		//	The body is encased in curly braces
		size_t end = i + tokens[i].length;
		i++;

		//	Parse the body
		result = parseScope(i, end, current);
	}

	//	There's no body so try to parse the next line
	else result = parseLine(i, current);

	i--;
	return result;
}
