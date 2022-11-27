#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

std::vector <std::string> readFile()
{
	std::vector <std::string> source;
	std::ifstream file("source");
	std::string line;

	while(std::getline(file, line))
		source.emplace_back(line + '\n');

	return source;
}

int main()
{
	auto source(readFile());
	size_t selection;

	std::vector <int> storage;
	int acc[2];

	for(auto& s : source)
	{
		std::stringstream ss(s);

		std::string cmd;
		size_t arg;

		ss >> cmd;
		ss >> arg;

		printf("cmd '%s' arg %lu\n", cmd.c_str(), arg);

		if(cmd == "select") selection = arg - 1;
		else if(cmd == "store") acc[selection] = arg;

		else if(cmd == "load")
		{
			if(arg >= storage.size())
				storage.push_back(0);

			acc[selection] = storage[arg];
		}

		else if(cmd == "save")
		{
			if(arg >= storage.size())
				storage.push_back(0);

			storage[arg] = acc[selection];
			printf("Saved %d to storage %lu\n", storage[arg], arg);
		}

		else if(cmd == "add") acc[selection] += arg;
		else if(cmd == "aadd") acc[selection] += acc[arg - 1];

		else if(cmd == "sub") acc[selection] -= arg;
		else if(cmd == "asub") acc[selection] -= acc[arg - 1];

		else if(cmd == "mul") acc[selection] *= arg;
		else if(cmd == "amul") acc[selection] *= acc[arg - 1];

		else if(cmd == "div") acc[selection] /= arg;
		else if(cmd == "adiv") acc[selection] /= acc[arg - 1];

		else if(cmd == "pow") acc[selection] = pow(acc[selection], arg);
		else if(cmd == "apow") acc[selection] = pow(acc[selection], acc[arg - 1]);

		else
		{
			printf("Invalid command '%s'\n", cmd.c_str());
			return 1;
		}
	}
}
