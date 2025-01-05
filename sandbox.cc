#include <cap/Client.hh>
#include <cap/Traverser.hh>
#include <cap/Function.hh>
#include <cap/Expression.hh>
#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Declaration.hh>
#include <cap/Value.hh>

#include <iostream>
#include <fstream>
#include <cassert>

class SourceString : public cap::Source
{
public:
	SourceString(std::wstring&& str)
		: str(std::move(str))
	{
	}

	wchar_t operator[](size_t index) const override
	{
		return str[index];
	}

	std::wstring getString(cap::Token token) const override
	{
		auto offset = str.begin() + token.getIndex();
		return std::wstring(offset, offset + token.getLength());
	}

	bool match(cap::Token token, std::wstring_view value) const override
	{
		return str.compare(token.getIndex(), token.getLength(), value) == 0;
	}

private:
	std::wstring str;
};

class Sandbox : public cap::Client
{
public:
	void onSourceError(cap::SourceLocation& at, const std::wstring& msg) override
	{
		printf("[%u:%u] Error in '%ls': %ls\n", at.getRow(), at.getColumn(), at.getString().c_str(), msg.c_str());
	}
};

class ASTDumper : public cap::Traverser
{
public:
	ASTDumper(std::string&& path)
		: file(path)
	{
		file << "@startmindmap\n";
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

		depth--;
	}

	Result onScope(std::shared_ptr <cap::Scope> node) override
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
		file << prefix() << node->getTypeString() << '\n';
		return Result::Continue;
	}

	Result onDeclaration(std::shared_ptr <cap::Declaration> node) override
	{
		file << prefix() << node->getTypeString() << '\n';
		return Result::Continue;
	}

	Result onBinaryOperator(std::shared_ptr <cap::BinaryOperator> node) override
	{
		file << prefix() << node->getTypeString() << '\n';
		return Result::Continue;
	}

	Result onValue(std::shared_ptr <cap::Value> node) override
	{
		file << prefix() << node->getTypeString() << ": " << node->getValue() << '\n';
		return Result::Continue;
	}

private:
	std::wstring prefix()
	{
		depth++;
		return std::wstring(depth, '*') + L' ';
	}

	unsigned depth = 0;
	std::wofstream file;
};

int main()
{
	// TODO: Define per source?
	std::locale::global(std::locale("C.UTF-8"));
    std::wcout.imbue(std::locale());

	Sandbox client;
	SourceString entry(LR"SRC(

		func main()
		{
			1+2
		}

	)SRC");

	client.parse(entry);

	ASTDumper dumper("ast.puml");
	dumper.traverseNode(entry.getGlobal());

	return 0;
}
