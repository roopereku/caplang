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
	static void error(const std::string& file, const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(file, nullptr, true);
	}

	template <typename... Args>
	static void error(const std::string& file, Token& at, const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(file, &at, true);
	}

	template <typename... Args>
	static void warning(const std::string& file, const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(file, nullptr, false);
	}

	template <typename... Args>
	static void warning(const std::string& file, Token& at, const char* fmt, Args&& ...args)
	{
		snprintf(buffer, sizeof(buffer), fmt, std::forward <Args> (args)...);
		addLog(file, &at, false);
	}

private:
	static void addLog(const std::string& file, Token* at, bool error);

	static char buffer[100];
	static std::vector <Log> logs;
};

}

#endif
