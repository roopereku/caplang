#include <cap/Attribute.hh>
#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/CallableType.hh>
#include <cap/ClassType.hh>
#include <cap/Client.hh>
#include <cap/Expression.hh>
#include <cap/Function.hh>
#include <cap/Identifier.hh>
#include <cap/Integer.hh>
#include <cap/Return.hh>
#include <cap/String.hh>
#include <cap/Traverser.hh>
#include <cap/TypeReference.hh>
#include <cap/UnaryOperator.hh>
#include <cap/Variable.hh>

#include <cassert>
#include <fstream>
#include <iostream>

class Sandbox : public cap::Client
{
public:
    void onSourceError(cap::SourceLocation& at, const std::wstring& msg) override
    {
        printf("[%u:%u] Error in '%ls': %ls\n", at.getRow(), at.getColumn(), at.getString().c_str(), msg.c_str());
    }

    void onDebugMessage(const std::wstring& msg) override { printf("Debug: %ls\n", msg.c_str()); }
};

class ASTDumper : public cap::Traverser
{
public:
    ASTDumper(std::string&& path, cap::Client& client) :
        m_file(path),
        m_client(client)
    {
        m_file << "@startmindmap\n";
        m_file << "<style>\n";
        m_file << "root {\nBackgroundColor #00000000;\n}\n";
        m_file << "element {\nBackgroundColor #BBBBBB; LineColor #BBBBBB;\n}\n";
        m_file << "</style>\n";
    }

    ~ASTDumper()
    {
        m_file << "@endmindmap\n";
        m_file.close();

        // NOTE: Enable this for visualization if you have plantuml and sxiv in your PATH.
#if 1
        system("plantuml ast.puml");
        system("sxiv ast.png");
#endif
    }

protected:
    void onNodeExited(std::shared_ptr<cap::Node> node, Result result) override
    {
        if (result == cap::Traverser::Result::NotHandled)
        {
            printf("No handler defined for '%s'\n", node->getTypeString());
            return;
        }

        m_depth--;
    }

    Result onScope(std::shared_ptr<cap::Scope> node) override
    {
        m_file << prefix() << node->getTypeString() << '\n';
        return Result::Continue;
    }

    Result onClassType(std::shared_ptr<cap::ClassType> node) override
    {
        m_file << prefix() << node->getTypeString() << ": " << node->getName() << '\n';
        return Result::Continue;
    }

    Result onFunction(std::shared_ptr<cap::Function> node) override
    {
        m_file << prefix() << node->getTypeString() << ": " << node->getName() << '\n';
        return Result::Continue;
    }

    Result onExpressionRoot(std::shared_ptr<cap::Expression::Root> node) override
    {
        m_file << prefix() << node->getTypeString() << getResultType(node) << '\n';
        return Result::Continue;
    }

    Result onVariable(std::shared_ptr<cap::Variable> node) override
    {
        m_file << prefix();
        if (node->isAttribute())
        {
            m_file << " Attribute ";
        }

        m_file << node->getTypeString() << " " << node->getName() << getResultType(node) << '\n';
        return Result::Continue;
    }

    Result onTypeReference(std::shared_ptr<cap::TypeReference> node) override
    {
        m_file << prefix() << node->getTypeString() << getResultType(node) << '\n';
        return Result::Continue;
    }

    Result onBinaryOperator(std::shared_ptr<cap::BinaryOperator> node) override
    {
        m_file << prefix() << node->getTypeString() << getResultType(node) << '\n';
        return Result::Continue;
    }

    Result onUnaryOperator(std::shared_ptr<cap::UnaryOperator> node) override
    {
        m_file << prefix() << node->getTypeString() << getResultType(node) << '\n';
        return Result::Continue;
    }

    Result onBracketOperator(std::shared_ptr<cap::BracketOperator> node) override
    {
        m_file << prefix() << node->getTypeString() << getResultType(node) << '\n';
        return Result::Continue;
    }

    Result onReturn(std::shared_ptr<cap::Return> node) override
    {
        m_file << prefix() << node->getTypeString() << '\n';
        return Result::Continue;
    }

    Result onIdentifier(std::shared_ptr<cap::Identifier> node) override
    {
        m_file << prefix() << node->getTypeString() << ": " << node->getValue() << getResultType(node);

        if (node->getReferred())
        {
            m_file << "\\nRefers -> " << node->getReferred()->getLocation();
        }

        m_file << '\n';
        return Result::Continue;
    }

    Result onInteger(std::shared_ptr<cap::Integer> node) override
    {
        m_file << prefix() << node->getTypeString() << ": " << node->getValue() << getResultType(node) << '\n';
        return Result::Continue;
    }

    Result onString(std::shared_ptr<cap::String> node) override
    {
        // TODO: If the string is interpolated, show the parts separately.
        m_file << prefix() << node->getTypeString() << ": " << node->getValue() << getResultType(node) << '\n';
        return Result::Continue;
    }

private:
    std::wstring getResultType(std::shared_ptr<cap::Node> node)
    {
        std::wstring str;
        std::optional<cap::TypeContext> result;

        if (node->getType() == cap::Node::Type::Expression)
        {
            auto exprResult = std::static_pointer_cast<cap::Expression>(node)->getResultType();
            if (exprResult)
            {
                result.emplace(*exprResult);
            }
        }

        else if (node->getType() == cap::Node::Type::Declaration)
        {
            auto declResult = std::static_pointer_cast<cap::Declaration>(node)->getReferredType();
            if (declResult)
            {
                result.emplace(*declResult);
            }
        }

        if (result)
        {
            str += L"\\nResult -> " + result->toString();
        }

        return str;
    }

    std::wstring prefix()
    {
        m_depth++;
        return std::wstring(m_depth, '*') + L' ';
    }

    unsigned m_depth = 0;
    std::wofstream m_file;

    cap::Client& m_client;
};

int main()
{
    // TODO: Define per source?
    std::locale::global(std::locale("C.UTF-8"));
    std::wcout.imbue(std::locale());

    Sandbox client;
    cap::Source entry(
        LR"SRC(

        func foo() { let a = 10 }

	)SRC");

    if (!client.parse(entry, true))
    {
        return 1;
    }

    ASTDumper dumper("ast.puml", client);
    dumper.traverseNode(entry.getGlobal());

    return 0;
}
