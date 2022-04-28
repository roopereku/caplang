#ifndef CAP_LOGGER_HH
#define CAP_LOGGER_HH

#include "Token.hh"

#include <string>
#include <vector>

namespace Cap {

struct Log
{
	Log(const std::string& file, Token* at, std::string&& msg, bool isError)
		: file(file), at(at), msg(std::move(msg)), isError(isError)
	{
	}

	std::string file;
	Token* at = nullptr;

	std::string msg;
	bool isError;
};

class Logger
{
public:
	template <typename... Args>
	static inline void error(const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(nullptr, true);
	}

	template <typename... Args>
	static inline void error(Token& at, const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(&at, true);
	}

	template <typename... Args>
	static inline void warning(const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(nullptr, false);
	}

	template <typename... Args>
	static inline void warning(Token& at, const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(&at, false);
	}

	static inline void setCurrentFile(const std::string& path)
	{
		currentFile = path;
	}

private:
	static void addLog(Token* at, bool error);

	static char buffer[100];
	static std::vector <Log> logs;
	static std::string currentFile;
};

}

#endif
