#include "TokenizedSource.hh"
#include "Debug.hh"

#include <fstream>
#include <cctype>

static bool isOperator(char c)
{
	return	c == '+' || c == '-' || c == '.' || c == '/' || c == '*' ||
			c == '!' || c >= '!' && c <= '&' || c == '^' ||
			c >= '<' && c <= '@' || c == '~' || c == '|';
}

static bool isBracket(char c)
{
	return	c == '{' || c == '(' || c == '[' ||
			c == '}' || c == ')' || c == ']';
}

static bool isString(char c)
{
	return c == '\'' || c == '"';
}

static bool isBreak(char c)
{
	return c == ',' || c == ';';
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
	DBG_LOG("Reading file '%s'", path);
	std::ifstream file(path);

	if(!file.is_open())
	{
		printf("Error: Couldn't open file '%s'\n", path);
		return;
	}

	//	Read the file with a single allocation
	file.seekg(0, std::ios::end);
	data.resize(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&data[0], data.size());
	file.close();

	tokenize();
}

void Cap::TokenizedSource::addToken(TokenType type, size_t begin, size_t end)
{
	Token t;
	t.type = type;
	t.line = line;
	t.column = column;
	t.begin = &data[begin];
	t.length = end - begin;

	DBG_LOG("Added token of type '%s' on line %u at column %u with value '%s'", t.getTypeString(), line, column, t.getString().c_str());
	tokens.push_back(t);
}

void Cap::TokenizedSource::tokenize()
{
	error = false;
	column = 1;
	line = 1;

	for(size_t i = 0; i < data.length(); i++)
	{
		if(	!parseIdentifier(i) && !parseOperator(i) &&
			!parseNumeric(i))
		{
			//	Move onto the next line
			if(data[i] == '\n')
			{
				line++;
				column = 1;
				continue;
			}

			else if(isspace(data[i]))
				continue;

			printf("Error: Invalid character '%c'\n", data[i]);
			error = true;
			break;
		}

		i--;
	}

	if(error)
	{
		data.clear();
		data.shrink_to_fit();

		return;
	}
}

bool Cap::TokenizedSource::parseIdentifier(size_t& i)
{
	size_t begin = i;
	for(; i < data.length() && !isspace(data[i]) && !isBreak(data[i]) &&
		  !isOperator(data[i]) && !isString(data[i]) &&
		 (!isdigit(data[i]) || i > begin); i++, column++);

	if(i > begin)
	{
		addToken(TokenType::Identifier, begin, i);
		return true;
	}

	return false;
}

bool Cap::TokenizedSource::parseOperator(size_t& i)
{
	size_t begin = i;
	for(; i < data.length() && isOperator(data[i]); i++, column++)
	{
		DBG_LOG("operator char '%c'", data[i]);
		addToken(TokenType::Operator, i, i + 1);
	}

	return i > begin;
}

bool Cap::TokenizedSource::parseNumeric(size_t& i)
{
	if(!isdigit(data[i]))
		return false;

	size_t begin = i;

	//	'0' at the beginning could indicate non-decimal
	if(data[begin] == '0')
	{
		//	Check if the following chaaracter specifies non-decimal
		char determinant = tolower(data[++i]);

		if(	determinant == 'x' && parseHexadecimal(i) ||
			determinant == 'b' && parseBinary(i))
		{
			return true;
		}
	}

	if(i == begin && parseDecimal(i))
		return true;

	//	If no true was returned but the index moved, we have junk data
	if(i > begin)
	{
		DBG_LOG("Parsing junk for '%s'", tokens.back().getTypeString());
		for(begin = i; i < data.length() && !isspace(data[i]) && !isBreak(data[i]) &&
					   !isOperator(data[i]) && !isString(data[i]) &&
		 			   !isdigit(data[i]); i++, column++);

		std::string junk(data.begin() + begin, data.begin() + i);
		printf("Error: Junk after %s value '%s' ('%s')\n", tokens.back().getTypeString(), tokens.back().getString().c_str(), junk.c_str());

		return errorOut();
	}

	return false;
}

bool Cap::TokenizedSource::parseDecimal(size_t& i)
{
	size_t begin = i;
	size_t dots = 0;

	for(; i < data.length(); i++)
	{
		DBG_LOG("integer '%c'", data[i]);

		if(data[i] == '.')
		{
			dots++;

			//	If there's 2 consecutive dots, consider it range syntax
			if(data[i + 1] == '.')
			{
				DBG_LOG("Range at line %u column %u", line, column);
				break;
			}

			//	If the dots aren't consecutive, error out
			else if(dots > 1)
			{
				printf("Error: Too many dots in a numeric literal\n");
				return errorOut();
			}
		}

		else if(!isdigit(data[i]))
			break;
	}

	addToken(TokenType::Integer, begin, i);
	return isspace(data[i]) || isOperator(data[i]) || isBracket(data[i]) || isString(data[i]);
}

bool Cap::TokenizedSource::parseHexadecimal(size_t& i)
{
	size_t begin = ++i;
	for(; i < data.length(); i++)
	{
		if(!isdigit(data[i]))
		{
			char value = tolower(data[i]);
			if(value < 'a' || value > 'f')
				break;
		}
	}

	addToken(TokenType::Hexadecimal, begin, i);
	return isspace(data[i]) || isOperator(data[i]) || isBracket(data[i]) || isString(data[i]);
}

bool Cap::TokenizedSource::parseBinary(size_t& i)
{
	size_t begin = ++i;
	for(; i < data.length(); i++)
	{
		if(data[i] != '0' && data[i] != '1')
			break;
	}

	addToken(TokenType::Binary, begin, i);
	return isspace(data[i]) || isOperator(data[i]) || isBracket(data[i]) || isString(data[i]);
}
