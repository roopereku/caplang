#include "TokenizedSource.hh"
#include "Debug.hh"

#include <fstream>
#include <cctype>

static bool isOperator(char c)
{
	return	c == '+' || c == '-' || c == '*' || c == '/' ||
			c == '!' || c == '&' || c == '|' || c == '^' ||
			c == '.' || c == '>' || c == '<' || c == '~' ||
			c == '?' || c == '%' || c == '=';
}

static decltype(Cap::Token::line) line;
static decltype(Cap::Token::column) column;
static bool error;

static bool errorOut()
{
	error = true;
	return true;
}

Cap::TokenizedSource::TokenizedSource(const char* path)
{
}

void Cap::TokenizedSource::addToken(TokenType type, size_t begin, size_t end)
{
	Token t;
	t.type = type;
	t.line = line;
	t.column = column;
	t.begin = &data[begin];
	t.length = end - begin;

	tokens.push_back(t);
}

void Cap::TokenizedSource::tokenize()
{
	error = false;
	for(size_t i = 0; i < data.length(); i++)
	{
		if(	parseIdentifier(i) ||
			parseOperator(i))
		{
			if(error)
			{
			}
		}

		//	Move onto the next line
		if(data[i] == '\n')
		{
			line++;
			column = 0;
		}
	}
}

bool Cap::TokenizedSource::parseIdentifier(size_t& i)
{
	size_t begin = i;
	while(	!isspace(data[i]) && !isOperator(data[i]) &&
			(!isdigit(data[i] || i > begin)))
	{
		DBG_LOG("identifier char '%c'", data[i]);
	}

	return true;
}

bool Cap::TokenizedSource::parseOperator(size_t& i)
{
	while(isOperator(data[i++]))
	{
		addToken(TokenType::Operator, i, i + 1);
		column++;
	}

	return false;
}
