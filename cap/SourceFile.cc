#include <cap/SourceFile.hh>
#include <cap/Tokenizer.hh>
#include <cap/Validator.hh>
#include <cap/Parser.hh>

#include <cap/event/DebugMessage.hh>

#include <fstream>

namespace cap
{

SourceFile::SourceFile()
{
}

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

SourceFile SourceFile::fromPath(std::string_view path)
{
	SourceFile sourceFile(path);
	return sourceFile;
}

SourceFile SourceFile::fromSource(std::string_view source)
{
	SourceFile sourceFile;
	sourceFile.data = source;

	return sourceFile;
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

	// Make sure that the primitive types are adopted by the shared scope.
	TypeDefinition::ensurePrimitivesAdopted();

	// Create the global scope for this source file and attach it to the shared scope.
	global = std::make_shared <ScopeDefinition> ();
	ScopeDefinition::getShared()->adopt(global);

	if(!parser.parse(tokens, global))
	{
		return false;
	}

	Validator validator(events);
	return validator.validate(global);
}

}
