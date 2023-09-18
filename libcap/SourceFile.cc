#include <cap/SourceFile.hh>
#include <cap/Tokenizer.hh>

#include <fstream>

namespace cap
{

SourceFile::SourceFile(std::string_view path)
{
	std::ifstream file(path.data());

	if(!file.is_open())
	{
		printf("Unable to open source file\n");
		return;
	}

	//	Read the file with a single allocation
	file.seekg(0, std::ios::end);
	source.resize(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&source[0], source.size());
	file.close();

	Tokenizer tokens(source);
	BraceMatcher braces;

	global.parse(tokens);
}

}
