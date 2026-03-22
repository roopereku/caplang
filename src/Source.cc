#include <cap/Client.hh>
#include <cap/Scope.hh>
#include <cap/Source.hh>
#include <cap/Validator.hh>

#include <cassert>
#include <string>

namespace cap
{

Source::Source(std::wstring&& src) :
    m_src(std::move(src))
{
}

bool Source::parse(Client& client, bool validate)
{
    // TODO: Add an actual error message when a source is already
    // parsed by some client. Add a test for this.
    assert(!m_global);
    m_global = std::make_shared<Scope>();
    auto builtin = client.getBuiltin();

    // Connect the global scope to the builtins.
    if (m_global != builtin.getGlobal())
    {
        builtin.getGlobal()->adopt(m_global);
    }

    Token::ParserContext tokenCtx(client, *this);
    Node::ParserContext nodeCtx(client, *this);

    Token currentToken = Token::parseFirst(tokenCtx);
    std::weak_ptr<Node> currentNode = m_global;

    while (currentToken.isValid())
    {
        // TODO: Associate comments with nodes.
        if (currentToken.getType() == Token::Type::Comment)
        {
            currentToken = Token::parseNext(tokenCtx, currentToken);
            continue;
        }

        DBG_MESSAGE(client, "Token '", getString(currentToken), "' (", currentToken.getTypeString(), ")");

        currentNode = currentNode.lock()->handleToken(nodeCtx, currentToken);
        if (currentNode.expired())
        {
            DBG_MESSAGE(client, "Failure");
            return false;
        }

        currentToken = Token::parseNext(tokenCtx, currentToken);
        nodeCtx.m_canOpenSubexpression = true;
        nodeCtx.m_exitedFrom = nullptr;
    }

    // The current node has to return to the root node.
    assert(currentNode.lock() == m_global);

    if (validate)
    {
        Validator validator(nodeCtx);
        if (!validator.traverseScope(m_global))
        {
            return false;
        }
    }

    return true;
}

std::shared_ptr<Scope> Source::getGlobal()
{
    return m_global;
}

const std::shared_ptr<Scope> Source::getGlobal() const
{
    return m_global;
}

wchar_t Source::operator[](size_t index) const
{
    return m_src[index];
}

std::wstring Source::getString(Token token) const
{
    auto offset = m_src.begin() + token.getIndex();
    return std::wstring(offset, offset + token.getLength());
}

bool Source::match(Token token, std::wstring_view value) const
{
    return m_src.compare(token.getIndex(), token.getLength(), value) == 0;
}

SourceLocation::SourceLocation(const Source& source, Token at) :
    m_source(source),
    m_at(at),
    m_row(1),
    m_column(1)
{
    for (size_t i = 0; i < at.getIndex(); i++)
    {
        // TODO: Handle windows linebreaks.
        if (source[i] == '\n')
        {
            m_row++;
            m_column = 1;
        }

        else
        {
            m_column++;
        }
    }
}

SourceLocation::SourceLocation(const SourceLocation& rhs) :
    m_source(rhs.m_source),
    m_at(rhs.m_at),
    m_row(rhs.m_row),
    m_column(rhs.m_column)
{
}

std::wstring SourceLocation::getString() const
{
    return m_source.getString(m_at);
}

unsigned SourceLocation::getRow() const
{
    return m_row;
}

unsigned SourceLocation::getColumn() const
{
    return m_column;
}

Token SourceLocation::getToken() const
{
    return m_at;
}

const Source& SourceLocation::getSource() const
{
    return m_source;
}

} // namespace cap
