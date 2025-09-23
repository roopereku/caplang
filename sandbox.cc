#include <cap/Client.hh>
#include <cap/Traverser.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/CallableType.hh>
#include <cap/Expression.hh>
#include <cap/BinaryOperator.hh>
#include <cap/UnaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/TypeReference.hh>
#include <cap/Variable.hh>
#include <cap/Return.hh>
#include <cap/Identifier.hh>
#include <cap/Integer.hh>
#include <cap/String.hh>
#include <cap/Attribute.hh>

#include <iostream>
#include <fstream>
#include <cassert>

class Sandbox : public cap::Client
{
public:
	void onSourceError(cap::SourceLocation& at, const std::wstring& msg) override
	{
		printf("[%u:%u] Error in '%ls': %ls\n", at.getRow(), at.getColumn(), at.getString().c_str(), msg.c_str());
	}

	void onDebugMessage(const std::wstring& msg) override
	{
		printf("Debug: %ls\n", msg.c_str());
	}
};

class ASTDumper : public cap::Traverser
{
public:
	ASTDumper(std::string&& path, cap::Client& client)
		: file(path), client(client)
	{
		file << "@startmindmap\n";
		file << "<style>\n";
		file << "root {\nBackgroundColor #00000000;\n}\n";
		file << "element {\nBackgroundColor #BBBBBB; LineColor #BBBBBB;\n}\n";
		file << "</style>\n";
	}

	~ASTDumper()
	{
		file << "@endmindmap\n";
		file.close();

	// NOTE: Enable this for visualization if you have plantuml and sxiv in your PATH.
#if 1
	system("plantuml ast.puml");
	system("sxiv ast.png");
#endif
	}

protected:
	void onNodeExited(std::shared_ptr <cap::Node> node, Result result) override
	{
		if(result == cap::Traverser::Result::NotHandled)
		{
			printf("No handler defined for '%s'\n", node->getTypeString());
			return;
		}

		getAttributes(node);

		depth--;
	}

	Result onScope(std::shared_ptr <cap::Scope> node) override
	{
		file << prefix() << node->getTypeString() << '\n';
		return Result::Continue;
	}

	Result onClassType(std::shared_ptr <cap::ClassType> node) override
	{
		file << prefix() << node->getTypeString() << ": " << node->getName() << '\n';
		return Result::Continue;
	}

	Result onFunction(std::shared_ptr <cap::Function> node) override
	{
		file << prefix() << node->getTypeString() << ": " << node->getName() << '\n';
		return Result::Continue;
	}

	Result onExpressionRoot(std::shared_ptr <cap::Expression::Root> node) override
	{
		file << prefix() << node->getTypeString() << getResultType(node) << '\n';
		return Result::Continue;
	}

	Result onVariable(std::shared_ptr <cap::Variable> node) override
	{
		file << prefix() << node->getTypeString() << " " << node->getName() << getResultType(node) << '\n';
		return Result::Continue;
	}

	Result onTypeReference(std::shared_ptr <cap::TypeReference> node) override
	{
		file << prefix() << node->getTypeString() << getResultType(node) << '\n';
		return Result::Continue;
	}

	Result onBinaryOperator(std::shared_ptr <cap::BinaryOperator> node) override
	{
		file << prefix() << node->getTypeString() << getResultType(node) << '\n';
		return Result::Continue;
	}

	Result onUnaryOperator(std::shared_ptr <cap::UnaryOperator> node) override
	{
		file << prefix() << node->getTypeString() << getResultType(node) << '\n';
		return Result::Continue;
	}

	Result onBracketOperator(std::shared_ptr <cap::BracketOperator> node) override
	{
		file << prefix() << node->getTypeString() << getResultType(node) << '\n';
		return Result::Continue;
	}

	Result onReturn(std::shared_ptr <cap::Return> node) override
	{
		file << prefix() << node->getTypeString() << '\n';
		return Result::Continue;
	}

	Result onIdentifier(std::shared_ptr <cap::Identifier> node) override
	{
		file << prefix() << node->getTypeString() << ": " << node->getValue() << getResultType(node);

		if(node->getReferred())
		{
			file << "\\nRefers -> " << node->getReferred()->getLocation();
		}

		file << '\n';
		return Result::Continue;
	}

	Result onInteger(std::shared_ptr <cap::Integer> node) override
	{
		file << prefix() << node->getTypeString() << ": " << node->getValue() << getResultType(node) << '\n';
		return Result::Continue;
	}

	Result onString(std::shared_ptr <cap::String> node) override
	{
		// TODO: If the string is interpolated, show the parts separately.
		file << prefix() << node->getTypeString() << ": " << node->getValue() << getResultType(node) << '\n';
		return Result::Continue;
	}

private:
	void getAttributes(std::shared_ptr <cap::Node> node)
	{
		if(node->getAttributeRange().second > 0)
		{
			//file << prefix() << "Attributes\n";

			for(auto& attribute : client.getAttributes(node))
			{
				traverseExpression(attribute->getFirst());
			}

			//depth--;
		}
	}

	std::wstring getResultType(std::shared_ptr <cap::Node> node)
	{
		std::wstring str;
        std::optional <cap::TypeContext> result;

		if(node->getType() == cap::Node::Type::Expression)
		{
            auto exprResult = std::static_pointer_cast <cap::Expression> (node)->getResultType();
			if(exprResult)
			{
				result.emplace(*exprResult);
			}
		}

		else if(node->getType() == cap::Node::Type::Declaration)
		{
            auto declResult = std::static_pointer_cast <cap::Declaration> (node)->getReferredType();
			if(declResult)
			{
				result.emplace(*declResult);
			}
		}

		if(result)
		{
			str += L"\\nResult -> " + result->toString();
		}

		return str;
	}

	std::wstring prefix()
	{
		depth++;
		return std::wstring(depth, '*') + L' ';
	}

	unsigned depth = 0;
	std::wofstream file;

	cap::Client& client;
};

int main()
{
	// TODO: Define per source?
	std::locale::global(std::locale("C.UTF-8"));
    std::wcout.imbue(std::locale());

	Sandbox client;
	cap::Source entry(LR"SRC(

		func main()
		{
			msg = @localized "StringId" + "_1"
			//ratio = @unsafe foo() + 10
			//@assume(result < 0) -param ** 100
			//@~nonii -getCallable("Name")().someField + 2

			// IDEA: attributes are not carried inside subexpressions
			//@unsafe a.b.c( /* Foo call is safe */ foo(), @unsafe bar())
			@unsafe1 a.b.c(foo(), @unsafe2 bar()).d
		}

	)SRC");

	if(!client.parse(entry, false))
	{
		return 1;
	}

	ASTDumper dumper("ast.puml", client);
	dumper.traverseNode(entry.getGlobal());

	return 0;
}
