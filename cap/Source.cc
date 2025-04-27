#include <cap/Source.hh>
#include <cap/Client.hh>
#include <cap/Validator.hh>

#include <string>
#include <cassert>

namespace cap
{

Source::Source()
{
}

bool Source::parse(Client& client, bool validate)
{
	// TODO: Add an actual error message when a source is already
	// parsed by some client. Add a test for this.
	assert(!global);
	global = std::make_shared <Scope> ();
	auto builtin = client.getBuiltin();

	// Connect the global scope to the builtins.
	if(global != builtin.getGlobal())
	{
		builtin.getGlobal()->adopt(global);
	}

	Token::ParserContext tokenCtx(client, *this);
	Node::ParserContext nodeCtx(client, *this);

	if(!canParse(client))
	{
		return false;
	}

	Token currentToken = Token::parseFirst(tokenCtx);
	std::weak_ptr <Node> currentNode = global;
	std::weak_ptr <Node> prevNode;

	while(currentToken.isValid())
	{
		// TODO: Associate comments with nodes.
		if(currentToken.getType() == Token::Type::Comment)
		{
			currentToken = Token::parseNext(tokenCtx, currentToken);
			continue;
		}

		DBG_MESSAGE(client, "Token '", getString(currentToken), "' (", currentToken.getTypeString(), ")");

		currentNode = currentNode.lock()->handleToken(nodeCtx, currentToken);
		if(currentNode.expired())
		{
			DBG_MESSAGE(client, "Failure");
			return false;
		}

		prevNode = currentNode;
		currentToken = Token::parseNext(tokenCtx, currentToken);
		nodeCtx.canOpenSubexpression = true;
		nodeCtx.exitedFrom = nullptr;
	}

	// The current node has to return to the root node.
	assert(currentNode.lock() == global);

	if(validate)
	{
		Validator validator(nodeCtx);
		if(!validator.traverseScope(global))
		{
			return false;
		}
	}

	return true;
}

std::shared_ptr <Scope> Source::getGlobal()
{
	return global;
}

const std::shared_ptr <Scope> Source::getGlobal() const
{
	return global;
}

bool Source::canParse(Client&)
{
	return true;
}

SourceLocation::SourceLocation(const Source& source, Token at)
	: source(source), at(at), row(1), column(1)
{
	for(size_t i = 0; i < at.getIndex(); i++)
	{
		// TODO: Handle windows linebreaks.
		if(source[i] == '\n')
		{
			row++;
			column = 1;
		}

		else
		{
			column++;
		}
	}
}

SourceLocation::SourceLocation(const SourceLocation& rhs)
	: source(rhs.source), at(rhs.at), row(rhs.row), column(rhs.column)
{
}

std::wstring SourceLocation::getString() const
{
	return source.getString(at);
}

unsigned SourceLocation::getRow() const
{
	return row;
}

unsigned SourceLocation::getColumn() const
{
	return column;
}

Token SourceLocation::getToken() const
{
	return at;
}

const Source& SourceLocation::getSource() const
{
	return source;
}

}
