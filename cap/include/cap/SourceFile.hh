#ifndef CAP_SOURCE_FILE_HH
#define CAP_SOURCE_FILE_HH

#include <cap/EventEmitter.hh>
#include <cap/node/ScopeDefinition.hh>

#include <string_view>
#include <string>

namespace cap
{

class SourceFile
{
public:
	/// SourceFile constructor.
	///
	/// \param path The path to a source file to load.
	SourceFile(std::string_view path);

	/// Loads a source file from the given path.
	///
	/// \param path The path to load a source file from.
	/// \return The created source file.
	static SourceFile fromPath(std::string_view path);

	/// Creates a source file with the given source code.
	///
	/// \param source The source code to use.
	/// \return The created source file.
	static SourceFile fromSource(std::string_view source);

	/// Parses and validates the loaded source file.
	///
	/// \param events The EventEmitter to pass events to.
	/// \return True if parsing was succesful.
	bool prepare(EventEmitter& events);

	/// Retrives the global scope node of this source file.
	/// \return The global scope node.
	std::shared_ptr <ScopeDefinition> getGlobal()
	{
		return global;
	}

	/// The path of this source file.
	const std::string_view path;

private:
	SourceFile();

	std::string data;
	std::shared_ptr <ScopeDefinition> global;
};

}

#endif
