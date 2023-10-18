#include <cap/SourceFile.hh>
#include <cap/EventEmitter.hh>
#include <cap/event/Message.hh>
#include <cap/PrimitiveType.hh>
#include <cap/node/Declaration.hh>
#include <cap/node/DeclarationReference.hh>
#include <cap/node/FunctionDeclaration.hh>
#include <cap/node/VariableDeclaration.hh>
#include <cap/node/TypeDeclaration.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TypedConstant.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/FunctionCall.hh>
#include <cap/node/Subscript.hh>
#include <cap/node/Operator.hh>
#include <cap/node/Statement.hh>
#include <cap/node/Return.hh>

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
	void generateNode(std::shared_ptr <cap::Node> node, unsigned depth)
	{
		//printf("%u '%s' %u\n", node->id, node->getToken().c_str(), depth);

		if(node->isExpression())
		{
			//printf("Expression node\n");
			auto expr = node->as <cap::Expression> ();
			std::string typeString("(" + expr->getResultType().getName().getString() + ")");

			if(expr->isOperator())
			{
				//printf("Operator\n");
				auto op = node->as <cap::Operator> ();

				if(op->isTwoSided())
				{
					//printf("Two sided operator\n");
					auto twoSided = op->as <cap::TwoSidedOperator> ();
					file << indent(depth) << twoSided->getTypeString() << ' ' << typeString << "\n";

					generateNode(twoSided->getLeft(), depth + 1);
					generateNode(twoSided->getRight(), depth + 1);
				}

				else if(op->isOneSided())
				{
					auto oneSided = op->as <cap::OneSidedOperator> ();
					file << indent(depth) << oneSided->getTypeString() << ' ' << typeString << "\n";

					if(oneSided->type == cap::OneSidedOperator::Type::FunctionCall)
					{
						//	printf("Function call %p\n", oneSided->getExpression().get());
						auto call = oneSided->as <cap::FunctionCall> ();
						generateNode(call->getParameters(), depth + 1);

						if(call->getExpression())
							generateNode(call->getExpression(), depth + 1);
					}

					else if(oneSided->type == cap::OneSidedOperator::Type::Subscript)
					{
						auto subscript = oneSided->as <cap::Subscript> ();
						generateNode(subscript->getContents(), depth + 1);

						if(subscript->getExpression())
							generateNode(subscript->getExpression(), depth + 1);
					}

					else generateNode(oneSided->getExpression(), depth + 1);
				}
			}

			else if(expr->isValue())
			{
				//printf("Value\n");
				file << indent(depth) << "Value: " << expr->getToken().getString() << "\n";
			}

			else if(expr->isExpressionRoot())
			{
				//printf("Expression root\n");
				file << indent(depth) << "Expression root " << typeString << '\n';

				if(!node->as <cap::ExpressionRoot> ()->getRoot())
				{
					//printf("No root\n");
				}

				else
				{
					generateNode(node->as <cap::ExpressionRoot> ()->getRoot(), depth + 1);
				}
			}

			else if(expr->isDeclarationReference())
			{
				auto ref = expr->as <cap::DeclarationReference> ();

				file << indent(depth) << "Reference to declaration \"" << ref->getDeclaration()->getName().getString() << "\" " << typeString << "\n";
			}

			else if(expr->isTypedConstant())
			{
				const auto& type = expr->getResultType();
				file << indent(depth) << "Constant \"" << expr->getToken().getString() << "\" " << typeString << '\n';
			}
		}

		else if(node->isDeclaration())
		{
			auto declNode = node->as <cap::Declaration> ();

			if(declNode->isFunction())
			{
				auto decl = node->as <cap::FunctionDeclaration> ();

				//printf("Function declaration\n");
				file << indent(depth) << "Function: " << decl->function->getName().getString() << "\n";
				generateNode(decl->function->getRoot(), depth + 1);
			}

			else if(declNode->isType())
			{
				auto decl = node->as <cap::TypeDeclaration> ();

				file << indent(depth) << "Type: " << decl->type->getName().getString() << "\n";
				generateNode(decl->type->getRoot(), depth + 1);
			}

			else if(declNode->isVariable())
			{
				auto decl = node->as <cap::VariableDeclaration> ();

				file << indent(depth) << "Variable declaration\n";
				generateNode(decl->initialization, depth + 1);
			}

			else if(declNode->isVariableDefinition())
			{
				auto decl = node->as <cap::VariableDeclaration> ();

				file << indent(depth) << "Variable definition\n";
				generateNode(decl->initialization, depth + 1);
			}
		}

		else if(node->isStatement())
		{
			auto statement = node->as <cap::Statement> ();
			file << indent(depth) << "Statement: " << statement->getToken().getString() << "\n";

			if(statement->isReturn())
			{
				generateNode(statement->as <cap::Return> ()->expression, depth + 1);
			}
		}

		if(node->hasNext())
		{
			generateNode(node->getNext(), depth);
		}
	}

	std::string indent(unsigned depth)
	{
		return std::string(depth, '*') + ' ';
	}

	std::ofstream file;
};

class StdoutEmitter : public cap::EventEmitter
{
public:
	void onMessageReceived(cap::Message& message) override
	{
		const char* prefix = "";

		switch(message.getType())
		{
			case cap::Message::Type::Error:
			{
				prefix = "\x1B[1;31mError";
				break;
			}
		}

		auto& location = message.getLocation();
		printf("[%s\x1B[0m:%lu:%lu] %s\n", prefix, location.getRow(), location.getColumn(), message.getString().c_str());
	}
};

int main()
{
	cap::SourceFile entry("../test.cap");
	StdoutEmitter events;

	if(!entry.parse(events))
	{
		printf("Parsing failed\n");
		return 1;
	}

	GraphGenerator gen("output");
	gen.generate(entry.getGlobal().getRoot());

	if(!entry.validate(events))
	{
		printf("Validation failed\n");
		return 1;
	}

	GraphGenerator validated("validated");
	validated.generate(entry.getGlobal().getRoot());
}
