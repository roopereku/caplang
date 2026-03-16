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
	: m_index(index), m_length(length)
{
}

Token::Token()
	: m_index(0), m_length(0), m_type(Type::Invalid)
{
}

size_t Token::getIndex() const
{
	return m_index;
}

size_t Token::getLength() const
{
	return m_length;
}

Token::Type Token::getType() const
{
	return m_type;
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
		case Type::Attribute: return "Attribute";
		case Type::Invalid: return "Invalid";
	}

	return "???";
}

const char* Token::getTypeString() const
{
	return Token::getTypeString(m_type);
}

bool Token::isValid() const
{
	return m_type != Type::Invalid;
}

bool Token::isReservedIdentifier(cap::ParserContext& ctx) const
{
	auto& source = ctx.m_source;

	return (
		source.match(*this, L"func") ||
		source.match(*this, L"type") ||
		source.match(*this, L"enum")
	);
}

bool Token::isOpeningBracket(cap::ParserContext& ctx, wchar_t ch) const
{
	return m_type == Type::OpeningBracket && ctx.m_source[m_index] == ch;
}

bool Token::isClosingBracket(cap::ParserContext& ctx, wchar_t ch) const
{
	return m_type == Type::ClosingBracket && ctx.m_source[m_index] == ch;
}

bool Token::isLastOfLine(cap::ParserContext& ctx)
{
	size_t i = m_index + m_length;
	while(true)
	{
		// TODO: Handle windows linebreaks.
		if(ctx.m_source[i] == '\n' || ctx.m_source[i] == 0)
		{
			return true;
		}

		// Check if a comment comes after this token.
		else if(ctx.m_source[i] == '/')
		{
			size_t temp = i;
			bool multiline;
			bool lineChanged;
			parseComment(ctx, temp, multiline, lineChanged);

			if(temp > i)
			{
				// If a comment after the token consumes the rest of the line,
				// this token is the last thing we care about on the current line.
				if(!multiline || lineChanged)
				{
					return true;
				}

				// Skip over the comment if there's something after it.
				i = temp;
			}

			else
			{
				break;
			}
		}

		else if(!isspace(ctx.m_source[i]))
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
	skipWhitespace(ctx, first.m_index);	
	return parse(ctx, first);
}

Token Token::parseNext(ParserContext& ctx, Token token)
{
	// Get the beginning of the next token.
	Token next = token;
	next.m_index += token.m_length;

	skipWhitespace(ctx, next.m_index);	
	ctx.m_previous = ctx.m_source[next.m_index - 1];
	return parse(ctx, next);
}

Token Token::parse(ParserContext& ctx, Token token)
{
	token.m_type = Type::Invalid;
	size_t i = token.m_index;

	// Every source should stop at a null terminator.
	if(ctx.m_source[i] == 0)
	{
		// All opening brackets must be terminated.
		if(!ctx.m_openedBrackets.empty())
		{
			wchar_t prevOpener = ctx.m_source[ctx.m_openedBrackets.top().m_index];
			SourceLocation location(ctx.m_source, ctx.m_openedBrackets.top());
			ctx.m_client.sourceError(location, "Unterminated bracket '", prevOpener, "'");
		}

		return token;
	}

	bool shorted = (
		token.setTypeIfMoved(ctx, i, &Token::parseBracket) ||
		token.setTypeIfMoved(ctx, i, &Token::parseNumeric) ||
		token.setTypeIfMoved(ctx, i, &Token::parseString) ||
		token.setTypeIfMoved(ctx, i, &Token::parseIdentifier) ||
		token.setTypeIfMoved(ctx, i, &Token::parseComment) ||
		token.setTypeIfMoved(ctx, i, &Token::parseOperator) ||
		token.setTypeIfMoved(ctx, i, &Token::parseAttribute)
	);

	if(!shorted && ctx.m_source[i] != 0)
	{
		token.m_length = 1;
		SourceLocation location(ctx.m_source, token);
		ctx.m_client.sourceError(location, "Invalid character");
	}

	return token;
}

void Token::skipWhitespace(ParserContext& ctx, size_t& i)
{
	wchar_t ch = ctx.m_source[i];
	while(ch != 0 && isspace(ch))
	{
		i++;
		ch = ctx.m_source[i];
	}
}

Token::ParseResult Token::parseBracket(ParserContext& ctx, size_t& i)
{
	wchar_t ch = ctx.m_source[i];
	wchar_t expectedOpener = 0;

	if(ch == '<')
	{
		// "<" is treated as an opening bracket if something immediately
		// follows it that doesn't make an operator.
		// Example: foo <5, 10>. foo < 5 would indicate an operator.
		wchar_t next = ctx.m_source[i + 1];
		if(next == '<' || next == '=' || isspace(next) || next == 0)
		{
			return Type::Invalid;
		}
	}

	switch(ch)
	{
		case '(': case '{': case '[': case '<':
		{
			ctx.m_openedBrackets.push(Token(i, 1));
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
			if(ctx.m_previous != '>' && !isspace(ctx.m_previous))
			{
				expectedOpener = '<';
				break;
			}

			return Type::Invalid;
		}

		// Not a bracket so move on to the next parser.
		default: return Type::Invalid;
	}

	Token at(i, 1);
	i++;

	// All closing brackets should be have a corresponding opening bracket.
	if(ctx.m_openedBrackets.empty())
	{
		SourceLocation location(ctx.m_source, at);
		ctx.m_client.sourceError(location, "Closing bracket '", ch, "' never opened");
		return Type::Invalid;
	}

	// TODO: Could the actual opener be cached?
	wchar_t actualOpener = ctx.m_source[ctx.m_openedBrackets.top().m_index];

	// All closing brackets must match theie corresponding opening bracket.
	if(actualOpener != expectedOpener)
	{
		SourceLocation location(ctx.m_source, at);
		ctx.m_client.sourceError(location, "Mismatching closing bracket '", ch, "'");
		return Type::Invalid;
	}

	ctx.m_openedBrackets.pop();
	return Type::ClosingBracket;
}

Token::ParseResult Token::parseIdentifier(ParserContext& ctx, size_t& i)
{
	while(isIdentifierCharacter(ctx.m_source[i]))
	{
		i++;
	}

	return Type::Identifier;
}

Token::ParseResult Token::parseOperator(ParserContext& ctx, size_t& i)
{
	static std::array <wchar_t, 17> opChars
	{
		'+', '-', '*', '/', '%', '*',
		'<', '>', '&', '|', '^', '~',
		'=', '!', '.', ',', ':'
	};

	while(std::find(opChars.begin(), opChars.end(), ctx.m_source[i]) != opChars.end())
	{
		i++;
	}

	return Type::Operator;
}

Token::ParseResult Token::parseComment(ParserContext& ctx, size_t& i)
{
	bool multiline;
	bool lineChanged;
	return parseComment(static_cast <cap::ParserContext&> (ctx), i, multiline, lineChanged);
}

Token::ParseResult Token::parseComment(cap::ParserContext& ctx, size_t& i, bool& isMultiline, bool& lineChanged)
{
	if(ctx.m_source[i] == '/')
	{
		i++;

		switch(ctx.m_source[i])
		{
			// Single line comment.
			case '/':
			{
				isMultiline = false;

				// TODO: Handle windows linebreaks.
				while(ctx.m_source[i] != '\n' && ctx.m_source[i] != 0)
				{
					i++;
				}

				break;
			}

			// Block comment.
			case '*':
			{
				isMultiline = true;

				std::wstring_view toMatch(L"*/");
				size_t current = 0;

				for(++i; current < toMatch.length() && ctx.m_source[i] != 0; i++)
				{
					// If a character matches, move on to the next one.
					if(ctx.m_source[i] == toMatch[current])
					{
						current++;
						continue;
					}

					// TODO: Handle windows linebreaks.
					else if(ctx.m_source[i] == '\n')
					{
						lineChanged = true;
					}

					current = 0;
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

	if(ctx.m_source[i] == '"')
	{
		bool escaped = false;
		bool matched = false;

		for(++i; !matched && ctx.m_source[i] != 0; i++)
		{
			// TODO: Handle windows linebreaks.
			// TODO: Support multiline strings.
			if(ctx.m_source[i] == '\n')
			{
				break;
			}

			// Exit on a non-escaped quote.
			matched = (ctx.m_source[i] == '"' && !escaped);

			// In case of consecutive backslashes, toggle to escape status.
			escaped = ctx.m_source[i] == '\\' ? !escaped : false;
		}

		if(!matched)
		{
			return ParseResult(L"Unterminated \"");
		}
	}

	return Type::String;
}

Token::ParseResult Token::parseAttribute(ParserContext& ctx, size_t& i)
{
	if(ctx.m_source[i] == '@')
	{
		i++;
	}

	return Type::Attribute;
}

Token::ParseResult Token::parseNumeric(ParserContext& ctx, size_t& i)
{
	if(ctx.m_source[i] == '0')
	{
		i++;
		switch(tolower(ctx.m_source[i]))
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
		if(isNumeric(ctx.m_source[i]))
		{
			i++;
			return parseOctal(ctx, i);
		}

		// If the next character wasn't a numeric value, the token is simply "0".
		return Type::Integer;
	}

	// TODO: Allow floats written as ".2f"
	else if(isNumeric(ctx.m_source[i]))
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
		wchar_t ch = ctx.m_source[i];

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
			if(isIdentifierCharacter(ctx.m_source[i]))
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
	wchar_t ch = tolower(ctx.m_source[i]);
	while(isNumeric(ch) || (ch >= 'a' && ch <= 'f'))
	{
		i++;
		ch = tolower(ctx.m_source[i]);
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
	while(ctx.m_source[i] == '0' || ctx.m_source[i] == '1')
	{
		i++;
	}

	if(isIdentifierCharacter(ctx.m_source[i]))
	{
		// TODO: Parse identifier to get the whole suffix.
		i++;
		return ParseResult(L"Invalid binary suffix");
	}

	return Type::Binary;
}

Token::ParseResult Token::parseOctal(ParserContext&, size_t&)
{
	// TODO: Implement octal.
	return Type::Invalid;
}

bool Token::setTypeIfMoved(ParserContext& ctx, size_t& i, ParseResult(Token::*callback)(ParserContext&, size_t&))
{
	size_t origin = i;
	auto result = (this->*callback)(ctx, i);
	m_length = i - m_index;

	if(!result.m_error.empty())
	{
		SourceLocation location(ctx.m_source, *this);
		ctx.m_client.sourceError(location, result.m_error);

		m_type = Type::Invalid;
		return true;
	}

	if(i != origin)
	{
		m_type = result.m_tokenType;
		return true;
	}

	return false;
}

}
