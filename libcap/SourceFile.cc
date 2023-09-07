#include <cap/SourceFile.hh>
#include <cap/Tokenizer.hh>

namespace cap
{

SourceFile::SourceFile(std::string_view path)
{
	Tokenizer tokens(path);
	BraceMatcher braces;

	global.parse(tokens);
}

}
