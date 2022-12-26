#include "cap/CodeGenerator.hh"
#include "cap/Program.hh"

int main(int argc, char** argv)
{
	(void)argc;

	Cap::CodeGenerator::setOutputType(Cap::CodeGenerator::Output::X86Intel);
	Cap::Program p(argv[1]);
}
