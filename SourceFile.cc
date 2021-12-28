#include "SourceFile.hh"

Cap::SourceFile::SourceFile(const std::string& path)
	: tokens(path), root(nullptr, ScopeContext::Block)
{
}

bool Cap::SourceFile::validate()
{
	return true;
}

const std::vector <Cap::Token*> Cap::SourceFile::getImports() const
{
	return imports;
}
