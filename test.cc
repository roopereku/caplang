#include <cap/SourceFile.hh>
#include <cap/event/Message.hh>

#include <cap/node/ScopeDefinition.hh>
#include <cap/node/Expression.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/ExpressionRoot.hh>

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

	void generateOperator(std::shared_ptr <cap::Operator> node, unsigned depth)
	{
		file << indent(depth) << "Operator: " << node->getTypeString() << '\n';

		switch(node->type)
		{
			case cap::Operator::Type::OneSided:
			{
				generateNode(node->as <cap::OneSidedOperator> ()->getExpression(), depth + 1);
				break;
			}

			case cap::Operator::Type::TwoSided:
			{
				generateNode(node->as <cap::TwoSidedOperator> ()->getLeft(), depth + 1);
				generateNode(node->as <cap::TwoSidedOperator> ()->getRight(), depth + 1);
				break;
			}
		}
	}

	void generateExpression(std::shared_ptr <cap::Expression> node, unsigned depth)
	{
		switch(node->type)
		{
			case cap::Expression::Type::Operator:
			{
				generateOperator(node->as <cap::Operator> (), depth);
				break;
			}

			case cap::Expression::Type::Value:
			{
				file << indent(depth) << "Value: " << node->token.getString() << '\n';
				break;
			}

			case cap::Expression::Type::Root:
			{
				file << indent(depth) << "Expression Root" << '\n';
				generateExpression(node->as <cap::ExpressionRoot> ()->getRoot(), depth + 1);
				break;
			}
		}
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

			case cap::Node::Type::Expression:
			{
				generateExpression(std::static_pointer_cast <cap::Expression> (node), depth);
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
