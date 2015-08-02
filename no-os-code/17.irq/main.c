#include "stdio.h"

void init_uart();

void print_cpsr(unsigned int cpsr, char *why)
{	
	printf("\n\rreason=%s cpsr = 0x%x\n\r", why, cpsr);

}

int main()
{
	int a, b;

	init_uart();
	
	while (1)
	{
		printf("please enter two number: \n\r");
		scanf("%d %d", &a, &b);
		printf("\n\r");
		printf("the sum of %d + %d is: %d\n\r", a, b, a+b);
	}
	
	return 0;
}
