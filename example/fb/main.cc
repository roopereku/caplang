#include <cstdio>

int main()
{
	for(int i = 0; i < 50; i++)
	{
		if(i % 5 == 0) printf("fizz");
		if(i % 3 == 0) printf("buzz");

		else printf("%d", i);

		putchar('\n');
	}
}
