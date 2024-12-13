#ifndef CAP_PARSER_CONTEXT_HH
#define CAP_PARSER_CONTEXT_HH

namespace cap
{

class Client;
class Source;

class ParserContext
{
public:
	ParserContext(Client& client, Source& source)
		: client(client), source(source)
	{
	}

	Client& client;
	Source& source;
};

}

#endif
