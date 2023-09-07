#include <cap/Tokenizer.hh>

#include <fstream>
#include <cctype>
#include <stack>

namespace cap
{

bool isIdentifierCharacter(char ch)
{
	return isalpha(ch) || ch == '_';
}

bool isOperatorCharacter(char ch)
{
	return
		ch == '!' || ch == '%' || ch == '&' ||
		ch == '^' || ch == '~' ||

		// * + , - . /
		(ch >= 42 && ch <= 47) ||

		// < = > ? @
		(ch >= 60 && ch < 64);
}

bool isBracket(char ch)
{
	return ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']';
}

Tokenizer::Tokenizer(std::string_view filePath)
{
	std::ifstream file(filePath.data());

	if(!file.is_open())
	{
		printf("Unable to open source file\n");
		return;
	}

	//	Read the file with a single allocation
	file.seekg(0, std::ios::end);
	data.resize(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&data[0], data.size());
	file.close();

	// Set the column to 1.
	// If the first character is a newline, row is incremented.
	updateCursorPosition();
}

Token Tokenizer::next()
{
	// If there are no more tokens, return an invalid token.
	if(empty())
	{
		return Token::createInvalid();
	}

	const size_t origin = index;
	const char ch = data[index];

	const size_t originRow = currentRow;
	const size_t originColumn = currentColumn;

	Token::Type tokenType = Token::Type::Invalid;

	if(isIdentifierCharacter(ch))
	{
		tokenType = parseIdentifier();
	}

	else if(isOperatorCharacter(ch))
	{
		tokenType = parseOperator();
	}

	else if(isdigit(ch))
	{
		tokenType = parseNumber();
	}

	else if(ch == '{' || ch == '}')
	{
		tokenType = Token::Type::CurlyBrace;
		nextCharacter();
	}

	else if(ch == '[' || ch == ']')
	{
		tokenType = Token::Type::SquareBracket;
		nextCharacter();
	}

	else if(ch == '(' || ch == ')')
	{
		tokenType = Token::Type::Parenthesis;
		nextCharacter();
	}

	else
	{
		printf("??? Weird character '%c'\n", ch);
	}

	// Save the string representing the token.
	std::string_view str(&data[origin], index - origin);

	// Skip trailing whitespace after the token.
	while(index < data.length() && isspace(data[index]))
		nextCharacter();

	if(tokenType == Token::Type::Invalid)
	{
		printf("Invalid token\n");
		std::quick_exit(1);
	}

	return Token(tokenType, std::move(str), originRow, originColumn);
}

bool Tokenizer::empty()
{
	return index >= data.size();
}

void Tokenizer::updateCursorPosition()
{
	const char ch = data[index];

	if(ch == '\n')
	{
		currentRow++;
		currentColumn = 1;
	}

	else currentColumn++;
}

bool Tokenizer::nextCharacter()
{
	index++;
	updateCursorPosition();

	return index < data.length();
}

Token::Type Tokenizer::parseIdentifier()
{
	while(!empty())
	{
		if(!(isIdentifierCharacter(data[index]) || isdigit(data[index])))
			break;

		nextCharacter();
	}

	return Token::Type::Identifier;
}

Token::Type Tokenizer::parseOperator()
{
	while(!empty() && isOperatorCharacter(data[index]))
	{
		nextCharacter();
	}

	return Token::Type::Operator;
}

Token::Type Tokenizer::parseNumber()
{
	unsigned dots = 0;

	// If the first character is 0, the token could be octal, hexadecimal or binary.
	if(data[index] == '0')
	{
		char ch = data[index];

		// If there's no next character, 0 is our token.
		if(!nextCharacter())
			return Token::Type::Integer;

		// If the next character is whitespace, stop parsing.
		if(isspace(data[index]))
			return Token::Type::Integer;

		// The token isn't a float if the next character isn't a dot.
		if(data[index] != '.')
		{
			printf("Unimplemented: Octal, Hex or binary\n");
			return Token::Type::Invalid;
		}

		else
		{
			if(!nextCharacter())
			{
				printf("Number ends after a dot\n");
				return Token::Type::Invalid;
			}

			dots++;
		}
	}

	while(!empty())
	{
		// Dots are allowed in numbers
		if(data[index] == '.')
			dots++;

		// If whitespace or an operator is encountered, stop parsing.
		else if(isspace(data[index]) || isOperatorCharacter(data[index]) || isBracket(data[index]))
			break;

		// If the character is something else, throw an error.
		else if(!isdigit(data[index]))
			return junkAfterNumber();

		nextCharacter();
	}

	if(dots > 1)
	{
		printf("Too many dots\n");
		return Token::Type::Invalid;
	}

	// If there are dots, the number is a float.
	return dots > 0 ? Token::Type::Float : Token::Type::Integer;
}

Token::Type Tokenizer::junkAfterNumber()
{
	printf("Junk after number\n");
	return Token::Type::Invalid;
}

}
