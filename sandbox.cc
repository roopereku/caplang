#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/Expression.hh>
#include <cap/BinaryOperator.hh>
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

class ASTDumper : public cap::Node::Traverser
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
	bool onFunction(std::shared_ptr <cap::Function> scope) override
	{
		depth++;
		file << prefix() << scope->getTypeString();

		if(!scope->getName().empty())
		{
			file << ": " << scope->getName();
		}

		file << '\n';

		for(auto nested : scope->getNested())
		{
			// TODO: Check return value?
			traverseNode(nested);
		}

		depth--;
		return false;
	}

	void onExpression(std::shared_ptr <cap::Expression> expr) override
	{
		depth++;

		switch(expr->getType())
		{
			case cap::Expression::Type::Root:
			{
				auto root = std::static_pointer_cast <cap::Expression::Root> (expr);
				file << prefix() << root->getTypeString() << '\n';

				traverseExpression(root->getFirst());
				break;
			}

			case cap::Expression::Type::BinaryOperator:
			{
				auto op = std::static_pointer_cast <cap::BinaryOperator> (expr);
				file << prefix() << op->getTypeString() << '\n';

				assert(op->getLeft());
				assert(op->getRight());

				traverseExpression(op->getLeft());
				traverseExpression(op->getRight());

				break;
			}

			case cap::Expression::Type::UnaryOperator:
			{
				break;
			}

			case cap::Expression::Type::Value:
			{
				auto value = std::static_pointer_cast <cap::Value> (expr);
				file << prefix() << "Value: " << value->getValue() << '\n';

				break;
			}
		}

		depth--;
	}

private:
	std::wstring prefix()
	{
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
			a = display.vga.offset >> 7 & 0xFFFF == b
		}

	)SRC");

	client.parse(entry);

	ASTDumper dumper("ast.puml");
	dumper.traverseNode(entry.getGlobal());

	return 0;
}
