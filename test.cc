#include "cap/CodeGenerator.hh"
#include "cap/Program.hh"

int main(int argc, char** argv)
{
	(void)argc;

	Cap::CodeGenerator::setOutput(Cap::CodeGenerator::Output::Test);
	Cap::Program p(argv[1]);
}
