#include <cap/Tokenizer.hh>

#include <string_view>
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

Tokenizer::Tokenizer(std::string& data) : data(data)
{
}

Token Tokenizer::next()
{
	// If there are no more tokens, return an invalid token.
	if(empty())
	{
		return Token::createInvalid();
	}

	origin = index;
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
		error = Error::InvalidCharacter;
	}

	size_t end = index;
	if(ignoreLastCharacter)
	{
		end--;
		ignoreLastCharacter = false;
	}

	// Save the string representing the token.
	std::string_view str(&data[origin], end - origin);

	// Skip trailing whitespace after the token.
	while(index < data.length() && isspace(data[index]))
		nextCharacter();

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
	// If the first character is 0, the token could be octal, hexadecimal or binary.
	if(data[index] == '0')
	{
		char ch = data[index];

		// If there's no next character, 0 is our token.
		if(!nextCharacter())
		{
			return Token::Type::Integer;
		}

		// If the next character is whitespace, stop parsing.
		if(isspace(data[index]))
		{
			return Token::Type::Integer;
		}

		switch(data[index])
		{
			// "0x" indicates hexadecimal.
			case 'x': case 'X':
			{
				if(!nextCharacter())
				{
					return junkAfterNumber();
				}

				return parseHexadecimal();
			}

			// "0b" indicates hexadecimal.
			case 'b': case 'B':
			{
				if(!nextCharacter())
				{
					return junkAfterNumber();
				}

				return parseBinary();
			}
		}
	}

	return parseIntegerOrFloat();
}

Token::Type Tokenizer::parseHexadecimal()
{
	// Skip "0x".
	size_t beforePrefix = origin;
	origin = index;

	while(!empty())
	{
		// If whitespace or an operator is encountered, stop parsing.
		if(isspace(data[index]) || isOperatorCharacter(data[index]) || isBracket(data[index]))
		{
			break;
		}

		else
		{
			// If the character is something other than a hex character, throw an error.
			char ch = tolower(data[index]);
			bool hexChar = (ch >= 'a' && ch <= 'f') || (ch >= '0' && ch <= '9');

			if(!hexChar)
			{
				origin = beforePrefix;
				return junkAfterNumber();
			}
		}

		nextCharacter();
	}

	// If no hexadecimal characters were processed, throw an error.
	if(index == origin)
	{
		origin = beforePrefix;
		return junkAfterNumber();
	}

	return Token::Type::Hexadecimal;
}

Token::Type Tokenizer::parseBinary()
{
	// Skip "0b".
	size_t beforePrefix = origin;
	origin = index;

	while(!empty())
	{
		// If whitespace or an operator is encountered, stop parsing.
		if(isspace(data[index]) || isOperatorCharacter(data[index]) || isBracket(data[index]))
		{
			break;
		}

		else
		{
			// If the character is something other than a binary character, throw an error.
			bool binaryChar = data[index] == '0' || data[index] == '1';

			if(!binaryChar)
			{
				origin = beforePrefix;
				return junkAfterNumber();
			}
		}

		nextCharacter();
	}

	// If no binary characters were processed, throw an error.
	if(index == origin)
	{
		origin = beforePrefix;
		return junkAfterNumber();
	}

	return Token::Type::Binary;
}

Token::Type Tokenizer::parseIntegerOrFloat()
{
	unsigned dots = 0;
	bool isFloat = false;

	while(!empty())
	{
		// Dots are allowed in numbers
		if(data[index] == '.')
		{
			dots++;
		}

		// If whitespace or an operator is encountered, stop parsing.
		else if(isspace(data[index]) || isOperatorCharacter(data[index]) || isBracket(data[index]))
		{
			break;
		}

		// If the character is something other than a number, throw an error.
		else if(!isdigit(data[index]))
		{
			// If the last character is "f", force the usage of float.
			if(tolower(data[index] == 'f'))
			{
				isFloat = true;
				nextCharacter();
				ignoreLastCharacter = true;
				break;
			}

			return junkAfterNumber();
		}

		nextCharacter();
	}

	if(dots > 1)
	{
		error = Error::TooManyDots;
		return Token::Type::Invalid;
	}

	if(dots > 0 || isFloat)
	{
		return isFloat ? Token::Type::Float : Token::Type::Double;
	}

	return Token::Type::Integer;
}

Token::Type Tokenizer::junkAfterNumber()
{
	error = Error::JunkAfterNumber;
	return Token::Type::Invalid;
}

void Tokenizer::reset()
{
	index = 0;
	currentRow = 1;
	currentColumn = 1;
}

Tokenizer::Error Tokenizer::getError()
{
	return error;
}

std::string_view Tokenizer::getErrorString()
{
	switch(error)
	{
		case Error::InvalidCharacter: return "Invalid character";
		case Error::JunkAfterNumber: return "Junk after a number";
		case Error::TooManyDots: return "Too many dots";
		case Error::None: return "None";
	}

	return "???";
}

}
