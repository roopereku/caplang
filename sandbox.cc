#include <cap/Client.hh>

#include <iostream>

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

int main()
{
	// TODO: Define per source?
	std::locale::global(std::locale("C.UTF-8"));
    std::wcout.imbue(std::locale());

	Sandbox client;

	SourceString entry(LR"SRC(
		
		func main()
		{
			1 + 2 * 3
		}

	)SRC");

	client.parse(entry);

	return 0;
}
