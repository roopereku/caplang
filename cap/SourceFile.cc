#include <cap/SourceFile.hh>
#include <cap/Tokenizer.hh>
#include <cap/Validator.hh>
#include <cap/Parser.hh>

#include <cap/event/DebugMessage.hh>

#include <fstream>

namespace cap
{

SourceFile::SourceFile(std::string_view path) : path(path)
{
	std::ifstream file(path.data());

	if(!file.is_open())
	{
		printf("Unable to open source file\n");
		return;
	}

	//	Read the file with a single allocation
	file.seekg(0, std::ios::end);
	data.resize(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(&data[0], data.size());
	file.close();
}

SourceFile::SourceFile(std::string& data) : data(data)
{
}

bool SourceFile::prepare(EventEmitter& events)
{
	// If the global scope node already exists, don't parse.
	if(global)
	{
		events.emit(DebugMessage(std::string("Root node already exists for ") + std::string(path), Token::createInvalid()));
		return false;
	}

	events.emit(DebugMessage(std::string("Parsing source file ") + std::string(path), Token::createInvalid()));

	Tokenizer tokens(data);
	Parser parser(events);

	global = std::make_shared <ScopeDefinition> ();
	if(!parser.parse(tokens, global))
	{
		return false;
	}

	Validator validator(events);
	return validator.validate(global);
}

}
