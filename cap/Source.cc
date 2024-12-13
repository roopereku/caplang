#include <cap/Source.hh>

#include <string>

namespace cap
{

Source::Source()
	: root(std::make_shared <Scope> ())
{
}

bool Source::parse(Client& client)
{
	Token::ParserContext tokenCtx(client, *this);
	Node::ParserContext nodeCtx(client, *this);

	if(!canParse(client))
	{
		return false;
	}

	Token currentToken = Token::parseFirst(tokenCtx);
	std::weak_ptr <Node> currentNode = root;
	std::weak_ptr <Node> prevNode;

	while(currentToken.isValid())
	{
		std::wstring str(getString(currentToken));
		printf("[%s] Token '%ls'\n", currentToken.getTypeString(), str.c_str());

		currentNode = currentNode.lock()->handleToken(nodeCtx, currentToken);
		if(currentNode.expired())
		{
			return false;
		}

		// If the previousNode isn't the same as the current one, the node has switched.
		// In such a case reset some state in the node parser context.
		// TODO: Maybe this can be improved?
		if(prevNode.expired() || currentNode.lock() != prevNode.lock())
		{
			nodeCtx.tokensProcessed = 0;
		}

		else
		{
			nodeCtx.tokensProcessed++;
		}

		prevNode = currentNode;
		currentToken = Token::parseNext(tokenCtx, currentToken);
	}

	return true;
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
	: at(rhs.at), source(rhs.source), row(rhs.row), column(rhs.column)
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
