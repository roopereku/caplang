#include "TokenizedSource.hh"
#include "Logger.hh"
#include "Debug.hh"

#include <fstream>
#include <cctype>

static bool isOperator(char c)
{
	return	c == '!' || c == '&' || (c >= '*' && c <= '/') ||
			c == '%' || c == '^' || (c >= '<' && c <= '@') ||
			c == '~' || c == '|';
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
	return c == ';';
}


bool Cap::TokenizedSource::errorOut()
{
	error = true;
	return true;
}

Cap::TokenizedSource::TokenizedSource(const std::string& path) : path(path)
{
	DBG_LOG("Reading file '%s'", path.c_str());
	Logger::setCurrentFile(path);
	std::ifstream file(path);

	if(!file.is_open())
	{
		Logger::error("Unable to open source file");
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

bool Cap::TokenizedSource::matchBraces()
{
	for(size_t i = 0; i < tokens.size(); i++)
	{
		char match;
		switch(tokens[i].type)
		{
			case TokenType::CurlyBrace: match = '}'; break;
			case TokenType::Parenthesis: match = ')'; break;
			case TokenType::SquareBracket: match = ']'; break;

			default: continue;
		}

		//	Is the current token a closing brace?
		if(*tokens[i].begin == match)
		{
			//	Does the closing brace have a match?
			if(tokens[i].length > 0)
			{
				Logger::error(tokens[i], "Unnecessary '%c'", match);
				return false;
			}
		}

		//	The current token is an opening brace but does it have a match?
		else if(!matchBrace(i, match))
			return false;
	}

	//	Let's make sure that there's nothing like "( { ) }"
	for(size_t i = 0; i < tokens.size(); i++)
	{
		//	Non-braces are irrelevant
		if(tokens[i].type < TokenType::CurlyBrace || tokens[i].type > TokenType::SquareBracket)
			continue;

		//	Where does this brace end
		size_t end = i + tokens[i].length;

		for(size_t j = i + 1; j < end; j++)
		{
			//	Non-braces are irrelevant
			if(tokens[j].type < TokenType::CurlyBrace || tokens[j].type > TokenType::SquareBracket)
				continue;

			//	Where does the inner brace end
			size_t innerEnd = j + tokens[j].length;

			//	If the inner brace is outside the outer brace, the braces are out of sequence
			if(innerEnd > end)
			{
				Logger::error(tokens[j], "Braces out of sequence");
				return false;
			}
		}
	}

	return true;
}

bool Cap::TokenizedSource::matchBrace(size_t i, char match)
{
	int depth = 1;
	size_t begin = i;

	for(i++; i < tokens.size() && depth > 0; i++)
	{
		//	Is the current token a brace?
		if(tokens[i].type == tokens[begin].type)
		{
			//	Does the current brace match the opening brace
			if(*tokens[i].begin == *tokens[begin].begin)
				depth++;

			//	Does the current brace match the closing brace
			else if(*tokens[i].begin == match)
				depth--;
		}
	}

	if(depth > 0)
	{
		Logger::error(tokens[begin], "Unterminated '%c'", *tokens[begin].begin);
		return false;
	}

	//	Tell the opening brace the range
	tokens[begin].length = --i - begin;
	tokens[i].length = 0;

	return true;
}

void Cap::TokenizedSource::addToken(TokenType type, size_t begin, size_t end)
{
	unsigned currentLine = current->line;
	unsigned currentColumn = current->column;

	current->type = type;
	current->line = startLine;
	current->column = startColumn;
	current->begin = &data[begin];
	current->length = end - begin;

	DBG_LOG("Added token of type '%s' on line %u at column %u with value '%s'", current->getTypeString(), startLine, startColumn, current->getString().c_str());
	tokens.push_back({});
	current = &tokens.back();

	startLine = current->line = currentLine;
	startColumn = current->column = currentColumn;
}

void Cap::TokenizedSource::tokenize()
{
	tokens.push_back({});
	current = &tokens.back();

	startColumn = current->column = 1;
	startLine = current->line = 1;
	error = false;

	for(size_t i = 0; i < data.length(); i++)
	{
		if(	!parseIdentifier(i) && !parseOperator(i) && !parseBracket(i) &&
			!parseNumeric(i) && !parseString(i) && !parseBreak(i))
		{
			//	Move onto the next line
			if(data[i] == '\n')
			{
				startLine = ++current->line;
				startColumn = current->column = 1;
				continue;
			}

			else if(isspace(data[i]))
			{
				startColumn = ++current->column;
				continue;
			}

			Logger::error(*current, "Invalid character '%c'", data[i]);
			error = true;
			break;
		}

		else if(error)
		{
			data.clear();
			data.shrink_to_fit();

			return;
		}

		i--;
	}
}

bool Cap::TokenizedSource::parseString(size_t& i)
{
	if(!isString(data[i]))
		return false;

	char match = data[i];
	size_t begin = ++i;

	for(bool escaped = false; i <= data.length(); i++, current->column++)
	{
		//	The string is unterminated if a newline or a null is encountered
		if(data[i] == '\n' || data[i] == 0)
		{
			current->line = startLine;
			current->column = startColumn;
			Logger::error(*current, "Unterminated %s", match == '"' ? "string" : "character");
			return errorOut();
		}

		//	If there's a match that isn't escaped, stop the loop
		if(data[i] == match && !escaped)
			break;

		escaped = data[i] == '\\' ? !escaped : false;
	}

	addToken(match == '"' ? TokenType::String : TokenType::Character, begin, i++);
	return true;
}

bool Cap::TokenizedSource::parseBracket(size_t& i)
{
	TokenType type;
	switch(data[i])
	{
		case '{': case '}': type = TokenType::CurlyBrace; break;
		case '(': case ')': type = TokenType::Parenthesis; break;
		case '[': case ']': type = TokenType::SquareBracket; break;

		default: return false;
	}

	addToken(type, i, i + 1);
	i++;
	return true;
}

bool Cap::TokenizedSource::parseBreak(size_t& i)
{
	if(!isBreak(data[i]))
		return false;

	addToken(TokenType::Break, i, i + 1);
	i++;
	return true;
}

bool Cap::TokenizedSource::parseSingleLineComment(size_t& i)
{
	//	Loop until the next line
	size_t begin = i += 2;
	for(; i < data.length() && data[i] != '\n'; i++, current->column++);

	addToken(TokenType::SingleLineComment, begin, i);
	return true;
}

bool Cap::TokenizedSource::parseMultiLineComment(size_t& i)
{
	const char* match = "*/";
	size_t begin = i += 2;
	size_t matched = 0;

	for(; i < data.length() && matched < 2; i++, current->column++)
	{
		if(data[i] == '\n')
		{
			current->line++;
			current->column = 1;
		}

		//	Move on to the next character to match if one matched
		if(data[i] == match[matched])
			matched++;

		//	Reset the match character if this one didn't match
		else matched = 0;
	}

	if(i >= data.length())
	{
		current->line = startLine;
		current->column = startColumn;
		Logger::error(*current, "Unterminated multiline comment");
		return errorOut();
	}

	addToken(TokenType::MultiLineComment, begin, i - 2);
	return true;
}

bool Cap::TokenizedSource::parseComment(size_t& i)
{
	if(data[i] != '/')
		return false;

	switch(data[i + 1])
	{
		case '/': return parseSingleLineComment(i);
		case '*': return parseMultiLineComment(i);
	}

	return false;
}

bool Cap::TokenizedSource::parseIdentifier(size_t& i)
{
	size_t begin = i;
	for(; i < data.length() && !isspace(data[i]) && !isBreak(data[i]) &&
		  !isOperator(data[i]) && !isBracket(data[i]) && !isString(data[i]) &&
		 (!isdigit(data[i]) || i > begin); i++, current->column++);

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
	for(; i < data.length() && isOperator(data[i]); i++, current->column++)
	{
		//	Since all comments use operators, check	for comments here
		if(parseComment(i)) return true;
		else addToken(TokenType::Operator, i, i + 1);
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
		//	Check if the following character specifies something non-decimal
		char determinant = tolower(data[i + 1]);

		if(	(determinant == 'x' && parseHexadecimal(i)) ||
			(determinant == 'b' && parseBinary(i)))
		{
			return true;
		}
	}

	//	If the index hasn't moved, check if there's a decimal value
	if(i == begin && parseDecimal(i))
		return true;

	/*	If any of the numeric parsers return false report the user
	 *	that there's junk after a valid numeric value */
	if(i > begin)
	{
		for(begin = i; i < data.length() && !isspace(data[i]) && !isBreak(data[i]) &&
					   !isOperator(data[i]) && !isString(data[i]) ; i++, current->column++);

		//	Without this validation there might be empty junk
		if(i != begin)
		{
			std::string junk(data.begin() + begin, data.begin() + i);
			Token& value = tokens[tokens.size() - 2];
			Logger::error(*current, "Junk after %s value '%s' ('%s')", value.getTypeString(), value.getString().c_str(), junk.c_str());

			return errorOut();
		}
	}

	return false;
}

bool Cap::TokenizedSource::parseDecimal(size_t& i)
{
	bool isFloat = false;
	size_t begin = i;
	size_t dots = 0;

	for(; i < data.length(); i++, current->column++)
	{
		if(data[i] == '.')
		{
			dots++;

			//	If there's 2 consecutive dots, consider it range syntax
			if(data[i + 1] == '.')
				break;

			//	If the dots aren't consecutive, error out
			else if(dots > 1)
			{
				Logger::error(*current, "Too many dots in a numeric literal");
				return errorOut();
			}
		}

		//	If there's a prefix that specifies float, 
		else if(data[i] == 'f' || data[i] == 'F')
		{
			isFloat = true;
			break;
		}

		else if(!isdigit(data[i]))
			break;
	}

	/*	For floats the index needs to be incremented after adding so that
	 *	the return check doesn't consider the 'f' to be junk data */
	if(isFloat) addToken(TokenType::Float, begin, i++);
	else addToken(dots == 0 ? TokenType::Integer : TokenType::Double, begin, i);

	return isspace(data[i]) || isOperator(data[i]) || isBracket(data[i]) || isString(data[i]);
}

bool Cap::TokenizedSource::parseHexadecimal(size_t& i)
{
	//	Loop while there are valid hexadecimal characters
	size_t begin = i += 2;
	for(; i < data.length(); i++, current->column++)
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
	//	Loop while there are valid binary characters
	size_t begin = i += 2;
	for(; i < data.length() && (data[i] == '1' || data[i] == '0'); i++, current->column++);

	addToken(TokenType::Binary, begin, i);
	return isspace(data[i]) || isOperator(data[i]) || isBracket(data[i]) || isString(data[i]);
}
