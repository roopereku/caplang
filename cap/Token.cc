#include <cap/Token.hh>
#include <cap/Client.hh>
#include <cap/Source.hh>
#include <cap/ParserContext.hh>

#include <algorithm>
#include <locale>
#include <cctype>
#include <array>

namespace cap
{

static bool isIdentifierCharacter(wchar_t ch)
{
	// TODO: The locale could be determined by the given Source.
	std::locale locale;
	return std::isalpha(ch, locale) || std::isdigit(ch, locale) || ch == '_';
}

static bool isNumeric(wchar_t ch)
{
	// TODO: The locale could be determined by the given Source.
	std::locale locale;
	return std::isdigit(ch, locale);
}

Token::Token(size_t index, size_t length)
	: index(index), length(length)
{
}

Token::Token()
	: index(0), length(0), type(Type::Invalid)
{
}

size_t Token::getIndex() const
{
	return index;
}

size_t Token::getLength() const
{
	return length;
}

Token::Type Token::getType() const
{
	return type;
}

const char* Token::getTypeString(Type type)
{
	switch(type)
	{
		case Type::OpeningBracket: return "OpeningBracket";
		case Type::ClosingBracket: return "ClosingBracket";
		case Type::Identifier: return "Identifier";
		case Type::Integer: return "Integer";
		case Type::Float: return "Float";
		case Type::Hexadecimal: return "Hexadecimal";
		case Type::Binary: return "Binary";
		case Type::Octal: return "Octal";
		case Type::String: return "String";
		case Type::Operator: return "Operator";
		case Type::Comment: return "Comment";
		case Type::Invalid: return "Invalid";
	}

	return "???";
}

const char* Token::getTypeString() const
{
	return Token::getTypeString(type);
}

bool Token::isValid() const
{
	return type != Type::Invalid;
}

bool Token::isReservedIdentifier(cap::ParserContext& ctx) const
{
	auto& source = ctx.source;

	return (
		source.match(*this, L"func") ||
		source.match(*this, L"type") ||
		source.match(*this, L"enum")
	);
}

bool Token::isOpeningBracket(cap::ParserContext& ctx, wchar_t ch) const
{
	return type == Type::OpeningBracket && ctx.source[index] == ch;
}

bool Token::isClosingBracket(cap::ParserContext& ctx, wchar_t ch) const
{
	return type == Type::ClosingBracket && ctx.source[index] == ch;
}

bool Token::isLastOfLine(cap::ParserContext& ctx)
{
	size_t i = index + length;
	while(true)
	{
		// TODO: Handle windows linebreaks.
		if(ctx.source[i] == '\n' || ctx.source[i] == 0)
		{
			return true;
		}

		else if(!isspace(ctx.source[i]))
		{
			break;
		}

		i++;
	}

	return false;
}

Token Token::parseFirst(ParserContext& ctx)
{
	Token first(0, 0);
	skipWhitespace(ctx, first.index);	
	return parse(ctx, first);
}

Token Token::parseNext(ParserContext& ctx, Token token)
{
	// Get the beginning of the next token.
	Token next = token;
	next.index += token.length;

	skipWhitespace(ctx, next.index);	
	ctx.previous = ctx.source[next.index - 1];
	return parse(ctx, next);
}

Token Token::parse(ParserContext& ctx, Token token)
{
	token.type = Type::Invalid;
	size_t i = token.index;

	// Every source should stop at a null terminator.
	if(ctx.source[i] == 0)
	{
		// All opening brackets must be terminated.
		if(!ctx.openedBrackets.empty())
		{
			wchar_t prevOpener = ctx.source[ctx.openedBrackets.top().index];
			SourceLocation location(ctx.source, ctx.openedBrackets.top());
			ctx.client.sourceError(location, "Unterminated bracket '", prevOpener, "'");
		}

		return token;
	}

	bool shorted = (
		token.setTypeIfMoved(ctx, i, &Token::parseBracket) ||
		token.setTypeIfMoved(ctx, i, &Token::parseNumeric) ||
		token.setTypeIfMoved(ctx, i, &Token::parseString) ||
		token.setTypeIfMoved(ctx, i, &Token::parseIdentifier) ||
		token.setTypeIfMoved(ctx, i, &Token::parseComment) ||
		token.setTypeIfMoved(ctx, i, &Token::parseOperator)
	);

	if(!shorted && ctx.source[i] != 0)
	{
		token.length = 1;
		SourceLocation location(ctx.source, token);
		ctx.client.sourceError(location, "Invalid character");
	}

	return token;
}

void Token::skipWhitespace(ParserContext& ctx, size_t& i)
{
	wchar_t ch = ctx.source[i];
	while(ch != 0 && isspace(ch))
	{
		i++;
		ch = ctx.source[i];
	}
}

Token::ParseResult Token::parseBracket(ParserContext& ctx, size_t& i)
{
	wchar_t ch = ctx.source[i];
	wchar_t expectedOpener = 0;

	switch(ch)
	{
		case '<':
		{
			// "<" is treated as an opening bracket if something immediately
			// follows it that doesn't make an operator.
			// Example: foo <5, 10>. foo < 5 would indicate an operator.
			wchar_t next = ctx.source[i + 1];
			if(next == '<' || next == '=' || isspace(next) || next == 0)
			{
				return Type::Invalid;
			}
		}

		case '(': case '{': case '[':
		{
			ctx.openedBrackets.push(Token(i, 1));
			i++;

			return Type::OpeningBracket;
		}

		case ')': expectedOpener = '('; break;
		case '}': expectedOpener = '{'; break;
		case ']': expectedOpener = '['; break;

		case '>':
		{
			// ">" is treated as a closing bracket if it immediately follows
			// something else. Example foo <5, 10> (x). 10 > (x) would indicate an operator.
			// TODO: Check for "-" to support "->" properly.
			if(ctx.previous != '>' && !isspace(ctx.previous))
			{
				expectedOpener = '<';
				break;
			}
		}

		// Not a bracket so move on to the next parser.
		default: return Type::Invalid;
	}

	Token at(i, 1);
	i++;

	// All closing brackets should be have a corresponding opening bracket.
	if(ctx.openedBrackets.empty())
	{
		SourceLocation location(ctx.source, at);
		ctx.client.sourceError(location, "Closing bracket '", ch, "' never opened");
		return Type::Invalid;
	}

	// TODO: Could the actual opener be cached?
	wchar_t actualOpener = ctx.source[ctx.openedBrackets.top().index];

	// All closing brackets must match theie corresponding opening bracket.
	if(actualOpener != expectedOpener)
	{
		SourceLocation location(ctx.source, at);
		ctx.client.sourceError(location, "Mismatching closing bracket '", ch, "'");
		return Type::Invalid;
	}

	ctx.openedBrackets.pop();
	return Type::ClosingBracket;
}

Token::ParseResult Token::parseIdentifier(ParserContext& ctx, size_t& i)
{
	while(isIdentifierCharacter(ctx.source[i]))
	{
		i++;
	}

	return Type::Identifier;
}

Token::ParseResult Token::parseOperator(ParserContext& ctx, size_t& i)
{
	static std::array <wchar_t, 16> opChars
	{
		'+', '-', '*', '/', '%', '*',
		'<', '>', '&', '|', '^',
		'=', '!', '.', ',', ':'
	};

	while(std::find(opChars.begin(), opChars.end(), ctx.source[i]) != opChars.end())
	{
		i++;
	}

	return Type::Operator;
}

Token::ParseResult Token::parseComment(ParserContext& ctx, size_t& i)
{
	if(ctx.source[i] == '/')
	{
		i++;

		switch(ctx.source[i])
		{
			// Single line comment.
			case '/':
			{
				// TODO: Handle windows linebreaks.
				while(ctx.source[i] != '\n' && ctx.source[i] != 0)
				{
					i++;
				}

				break;
			}

			// Block comment.
			case '*':
			{
				// TODO: If source ever contains std::wstring, use std::wstring::find?

				std::wstring_view toMatch(L"*/");
				size_t current = 0;

				for(++i; current < toMatch.length() && ctx.source[i] != 0; i++)
				{
					// For each matching character, move on to the next character or reset.
					current = (ctx.source[i] == toMatch[current]) ? current + 1 : 0;
				}

				// Unterminated multiline comment.
				if(current < toMatch.length())
				{
					return ParseResult(L"Unterminated comment");
				}

				break;
			}

			default: i--;
		}
	}

	return Type::Comment;
}

Token::ParseResult Token::parseString(ParserContext& ctx, size_t& i)
{
	// TODO: String prefixes?

	if(ctx.source[i] == '"')
	{
		bool escaped = false;
		bool matched = false;

		for(++i; !matched && ctx.source[i] != 0; i++)
		{
			// TODO: Handle windows linebreaks.
			// TODO: Support multiline strings.
			if(ctx.source[i] == '\n')
			{
				break;
			}

			// Exit on a non-escaped quote.
			matched = (ctx.source[i] == '"' && !escaped);

			// In case of consecutive backslashes, toggle to escape status.
			escaped = ctx.source[i] == '\\' ? !escaped : false;
		}

		if(!matched)
		{
			return ParseResult(L"Unterminated \"");
		}
	}

	return Type::String;
}

Token::ParseResult Token::parseNumeric(ParserContext& ctx, size_t& i)
{
	if(ctx.source[i] == '0')
	{
		i++;
		switch(tolower(ctx.source[i]))
		{
			// Hexadecimal.
			case 'x':
			{
				i++;
				return parseHexadecimal(ctx, i);
			}

			// Binary.
			case 'b':
			{
				i++;
				return parseBinary(ctx, i);
			}
		}

		// Any numeric value after "0" indicates octal.
		if(isNumeric(ctx.source[i]))
		{
			i++;
			return parseOctal(ctx, i);
		}

		// If the next character wasn't a numeric value, the token is simply "0".
		return Type::Integer;
	}

	// TODO: Allow floats written as ".2f"
	else if(isNumeric(ctx.source[i]))
	{
		return parseDecimal(ctx, i);
	}

	return Type::Invalid;
}

Token::ParseResult Token::parseDecimal(ParserContext& ctx, size_t& i)
{
	size_t dots = 0;
	wchar_t previous = 0;

	for(; i != 0; i++)
	{
		wchar_t ch = ctx.source[i];

		if(ch == '.')
		{
			// Make sure that operator ".." is treated separately.
			if(previous == '.')
			{
				dots--;
				i--;
				break;
			}

			// Only include a single dot in decimal literals.
		   if(++dots > 1)
		   {
			   break;
		   }
		}

		else if(!isNumeric(ch))
		{
			if(isIdentifierCharacter(ctx.source[i]))
			{
				// TODO: Parse identifier to get the whole suffix.
				return ParseResult(L"Invalid decimal suffix");
			}

			break;
		}

		previous = ch;
	}

	// TODO: Handle too many dots.	

	// TODO: If the decimal ends in a dot, treat the dot as its own operator.
	return dots > 0 ? Type::Float : Type::Integer;
}

Token::ParseResult Token::parseHexadecimal(ParserContext& ctx, size_t& i)
{
	wchar_t ch = tolower(ctx.source[i]);
	while(isNumeric(ch) || (ch >= 'a' && ch <= 'f'))
	{
		i++;
		ch = tolower(ctx.source[i]);
	}

	if(isIdentifierCharacter(ch))
	{
		// TODO: Parse identifier to get the whole suffix.
		i++;
		return ParseResult(L"Invalid hexadecimal suffix");
	}

	return Type::Hexadecimal;
}

Token::ParseResult Token::parseBinary(ParserContext& ctx, size_t& i)
{
	while(ctx.source[i] == '0' || ctx.source[i] == '1')
	{
		i++;
	}

	if(isIdentifierCharacter(ctx.source[i]))
	{
		// TODO: Parse identifier to get the whole suffix.
		i++;
		return ParseResult(L"Invalid binary suffix");
	}

	return Type::Binary;
}

Token::ParseResult Token::parseOctal(ParserContext& ctx, size_t& i)
{
	// TODO: Implement octal.
	return Type::Invalid;
}

bool Token::setTypeIfMoved(ParserContext& ctx, size_t& i, ParseResult(Token::*callback)(ParserContext&, size_t&))
{
	size_t origin = i;
	auto result = (this->*callback)(ctx, i);
	length = i - index;

	if(!result.error.empty())
	{
		SourceLocation location(ctx.source, *this);
		ctx.client.sourceError(location, result.error);

		type = Type::Invalid;
		return true;
	}

	if(i != origin)
	{
		type = result.tokenType;
		return true;
	}

	return false;
}

}
