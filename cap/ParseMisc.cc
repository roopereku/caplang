#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseMisc(size_t& i, Scope& current)
{
	SyntaxTreeNode::Type miscType;

	if(tokens[i].stringEquals("if"))
	{
		if(inExpression) return true;
		miscType = SyntaxTreeNode::Type::If;
	}

	else if(tokens[i].stringEquals("while"))
	{
		if(inExpression) return true;
		miscType = SyntaxTreeNode::Type::While;
	}

	else if(tokens[i].stringEquals("return"))
	{
		if(inExpression) return true;
		miscType = SyntaxTreeNode::Type::Return;
	}

	else if(tokens[i].stringEquals("when"))
	{
		if(inExpression) return true;
		miscType = SyntaxTreeNode::Type::When;
	}

	else return false;

	if(!current.parent || current.ctx == ScopeContext::Type)
	{
		Logger::error(tokens[i], "Can't use '%s' %s", tokens[i].getString().c_str(), !current.parent ? "in the global scope" : "inside a type");
		return errorOut();
	}

	Scope& scope = current.addBlock(ScopeContext::Block);

	Token& old = tokens[i];
	i++;

	if(!isToken(TokenType::Parenthesis, i))
	{
		Logger::error(tokens[i], "Expected parenthesis after '%s'", old.getString().c_str());
		return errorOut();
	}

	//	Store the index of the scope we're creating to the left node
	old.length = current.getBlockCount();
	current.node->type = miscType;
	current.node->value = &old;
	current.node->right = std::make_shared <SyntaxTreeNode> (current.node, nullptr, SyntaxTreeNode::Type::Expression);
	current.node = current.node->right.get();

	i++;
	scope.begin = i;
	scope.end = current.end;

	scope.root.left = std::make_shared <SyntaxTreeNode> (&scope.root, nullptr, SyntaxTreeNode::Type::Expression);
	scope.root.right = std::make_shared <SyntaxTreeNode> (&scope.root, nullptr, SyntaxTreeNode::Type::Expression);
	scope.node = scope.root.left.get();

	if(!parseLine(i, scope, true))
		return true;

	i++;
	scope.node = scope.root.right.get();

	if(!isToken(TokenType::CurlyBrace, i) || *tokens[i].begin == '}')
	{
		Logger::error(tokens[i], "Expected a body for '%s'", old.getString().c_str());
		return errorOut();
	}

	i++;
	if(!parseLine(i, scope, false))
		return true;

	return true;
}
