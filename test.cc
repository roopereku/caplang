#include <cap/SourceFile.hh>
#include <cap/event/Message.hh>

#include <cap/node/ScopeDefinition.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/VariableDefinition.hh>
#include <cap/node/ParameterDefinition.hh>
#include <cap/node/GenericInstantiation.hh>
#include <cap/node/ReturnStatement.hh>
#include <cap/node/CallOperator.hh>

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

	void generateOneSidedOperator(std::shared_ptr <cap::OneSidedOperator> node, unsigned depth)
	{
		if(node->type == cap::OneSidedOperator::Type::Call)
		{
			generateNode(node->as <cap::CallOperator> ()->getTarget(), depth + 1);
		}

		generateNode(node->getExpression(), depth + 1);
	}

	void generateOperator(std::shared_ptr <cap::Operator> node, unsigned depth)
	{
		if(node->getResultType().expired())
		{
			file << indent(depth) << node->getTypeString() << '\n';
		}

		else
		{
			file << indent(depth) << node->getTypeString() << " (" << node->getResultType().lock()->name.getString() << ")\n";
		}

		switch(node->type)
		{
			case cap::Operator::Type::OneSided:
			{
				generateOneSidedOperator(node->as <cap::OneSidedOperator> (), depth);
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

	void generateExpressionRoot(std::shared_ptr <cap::ExpressionRoot> node, unsigned depth)
	{
		switch(node->type)
		{
			case cap::ExpressionRoot::Type::Expression:
			{
				file << indent(depth) << "Expression" << '\n';
				break;
			}

			case cap::ExpressionRoot::Type::ReturnStatement:
			{
				file << indent(depth) << "Return"
					<< " (" << node->getResultType().lock()->name.getString() << ")\n";
				break;
			}

			case cap::ExpressionRoot::Type::InitializationRoot:
			{
				file << indent(depth) << "Initialization: " << node->token.getString() << '\n';
				break;
			}

			case cap::ExpressionRoot::Type::ExplicitReturnType:
			{
				if(node->getResultType().expired())
				{
					file << indent(depth) << "Explicit return type\n";
				}

				else
				{
					file << indent(depth) << "Explicit return type"
						<< " (" << node->getResultType().lock()->name.getString() << ")\n";
				}

				break;
			}

			case cap::ExpressionRoot::Type::VariableDefinition:
			{
				if(node->getResultType().expired())
				{
					file << indent(depth) << "Variable: " << node->as <cap::VariableDefinition> ()->name->token.getString() << '\n';
				}

				else
				{
					file << indent(depth) << "Variable: " << node->as <cap::VariableDefinition> ()->name->token.getString()
						<< " (" << node->getResultType().lock()->name.getString() << ")\n";
				}

				break;
			}

			case cap::ExpressionRoot::Type::ParameterDefinition:
			{
				if(node->getResultType().expired())
				{
					file << indent(depth) << "Parameter: " << node->as <cap::ParameterDefinition> ()->name->token.getString() << '\n';
				}

				else
				{
					file << indent(depth) << "Parameter: " << node->as <cap::ParameterDefinition> ()->name->token.getString()
						<< " (" << node->getResultType().lock()->name.getString() << ")\n";
				}

				break;
			}
		}

		generateNode(node->getRoot(), depth + 1);
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
				if(node->as <cap::Value> ()->isGeneric())
				{
					file << indent(depth) << "Generic value\n";

					generateExpression(node->as <cap::GenericInstantiation> ()->target, depth + 1);
					generateExpression(node->as <cap::GenericInstantiation> ()->argument, depth + 1);
				}

				else if(node->getResultType().expired())
				{
					file << indent(depth) << node->token.getTypeString() << ": " << node->token.getString() <<
							" (No type set)\n";
				}

				else
				{
					file << indent(depth) << node->token.getTypeString() << ": " << node->token.getString() <<
							" (" << node->getResultType().lock()->name.getString() << ")\n";
				}

				break;
			}

			case cap::Expression::Type::Root:
			{
				generateExpressionRoot(node->as <cap::ExpressionRoot> (), depth);
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

	if(!file.prepare(events))
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
