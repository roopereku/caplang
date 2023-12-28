#include <cap/SourceFile.hh>
#include <cap/event/Message.hh>

#include <cap/node/ScopeDefinition.hh>

#include <iostream>
#include <fstream>

class GraphGenerator
{
public:
	GraphGenerator(std::string_view path) : file(path.data())
	{
		file << "@startmindmap\n";
	}

	~GraphGenerator()
	{
		file << "@endmindmap\n";
	}

	void generate(std::shared_ptr <cap::Node> root)
	{
		generateNode(root, 1);
	}

private:
	void generateScope(std::shared_ptr <cap::ScopeDefinition> node, unsigned depth)
	{
		switch(node->type)
		{
			case cap::ScopeDefinition::Type::FunctionDefinition:
			{
				file << indent(depth) << "Function: " << node->name.getString() << '\n';
				break;
			}

			case cap::ScopeDefinition::Type::TypeDefinition:
			{
				file << indent(depth) << "Type: " << node->name.getString() << '\n';
				break;
			}

			case cap::ScopeDefinition::Type::None:
			{
				file << indent(depth) << "Scope\n";
				break;
			}
		}

		generateNode(node->getRoot(), depth + 1);
	}

	void generateNode(std::shared_ptr <cap::Node> node, unsigned depth)
	{
		if(!node)
		{
			return;
		}

		switch(node->type)
		{
			case cap::Node::Type::Empty:
			{
				file << indent(depth) << "Empty node\n";
				break;
			}

			case cap::Node::Type::ScopeDefinition:
			{
				generateScope(std::static_pointer_cast <cap::ScopeDefinition> (node), depth);
				break;
			}
		}

		generateNode(node->getNext(), depth);
	}

	std::string indent(unsigned depth)
	{
		return std::string(depth, '*') + ' ';
	}

	std::ofstream file;
};

class EventLogger : public cap::EventEmitter
{
public:
	void emit(cap::Event&& event) override
	{
		if(event == cap::Event::Type::Message)
		{
			const auto& message = event.as <cap::Message> ();
			const char* prefix = "[ ";

			switch(message.type)
			{
				case cap::Message::Type::Debug: prefix = "[ DEBUG "; break;
				case cap::Message::Type::Error: prefix = "[ ERROR "; break;
			}

			std::cout << prefix << event.at.getRow() << ':' << event.at.getColumn() << " ] " << message.contents << '\n';
		}
	}
};

int main()
{
	cap::SourceFile file("../test.cap");
	EventLogger events;

	if(!file.parse(events))
	{
		return 1;
	}

	{
		GraphGenerator graph("output");
		graph.generate(file.getGlobal());
	}

	system("plantuml output && sxiv .");

	return 0;
}
