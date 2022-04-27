#include "Logger.hh"

char Cap::Logger::buffer[100];
std::vector <Cap::Log> Cap::Logger::logs;

void Cap::Logger::addLog(const std::string& file, Token* at, bool error)
{
	if(error) fprintf(stderr, "[\u001b[31;1mError in %s", file.c_str());
	else fprintf(stderr, "[\u001b[33;1mWarning in %s", file.c_str());
	if(at) fprintf(stderr, ":%u:%u", at->line, at->column);

	fprintf(stderr, "\u001b[0m] %s\n", buffer);
	logs.emplace_back(file, at, std::string(buffer), error);
}
