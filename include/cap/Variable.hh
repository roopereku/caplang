#ifndef CAP_VARIABLE_HH
#define CAP_VARIABLE_HH

#include <cap/Declaration.hh>
#include <cap/Statement.hh>

#include <vector>

namespace cap
{

class BinaryOperator;
class Identifier;

class Variable : public Declaration
{
public:
    enum class Type
    {
        Local,
        Generic,
        Parameter,
    };

    Variable(Type type, std::shared_ptr<Identifier> name, std::shared_ptr<BinaryOperator> initialization);

    /// Validates the initialization of this variable.
    ///
    /// \param validator The validator used for traversal.
    /// \return True if validation succeeded.
    bool validate(Validator& validator) override;

    /// Gets the right side of the initializing binary operator.
    ///
    /// \return The expression initializing this variable.
    std::shared_ptr<Expression> getInitialization();

    static const char* getTypeString(Type type);
    const char* getTypeString() const override;

    class Root;

private:
    Type m_type;
    std::weak_ptr<BinaryOperator> m_initialization;
};

class Variable::Root : public Statement
{
public:
    Root(Variable::Type type);

    /// Creates the initializer expression and delegates the token forward.
    ///
    /// \param ctx The parser context.
    /// \param token The token to handle.
    /// \return The initializer expression.
    std::weak_ptr<Node> handleToken(Node::ParserContext& ctx, Token& token) override;

    /// Creates the variables and exits out of the variable root.
    ///
    /// \param ctx The parser context containing the exiting node.
    /// \param token The token that caused the exit.
    /// \return The parent node or null upon an invalid declaration.
    std::weak_ptr<Node> invokedNodeExited(Node::ParserContext& ctx, Token& token) override;

    /// Gets the expression root representing the variable initializer.
    ///
    /// \return The expression root representing the initializer.
    std::shared_ptr<Expression::Root> getInitializer() const;

    /// Gets the type of this declaration root.
    ///
    /// \return The type of this declaration root.
    Variable::Type getType() const;

    const char* getTypeString() const override;

    /// Iterators for accessing declarations declared through this root.
    auto begin() { return m_declared.begin(); }

    auto end() { return m_declared.end(); }

    auto cbegin() const { return m_declared.cbegin(); }

    auto cend() const { return m_declared.cend(); }

protected:
    /// Makes sure that something will be declared when not declaring parameters.
    bool onInitialize(cap::ParserContext& ctx, bool expectsTokens) override;

    bool requiresDeclaration(cap::ParserContext& ctx);

private:
    std::vector<std::shared_ptr<Declaration>> m_declared;
    std::shared_ptr<Expression::Root> m_initializer;
    Variable::Type m_type;
};

} // namespace cap

#endif
