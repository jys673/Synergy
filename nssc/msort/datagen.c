/*-------------------------------------------------------------------
    This program generates a sequence of random integers. The size 
    of the sequence is determined by the command line argument.

    Usage:   datagen  size

    The output file is named "SORT.DAT". Note that this is a case
    sensitive system.
--------------------------------------------------------------------*/ 
#include <stdio.h>
main(argc, argv)
int argc;
char **argv; 
{
	FILE *output;
	int  size, i, num;

	if (argc <= 1) {
		printf(" Usage:  datagen size \n");
		exit(1);
	}
	output = fopen("SORT.DAT", "w"); 
	size = atoi(argv[1]);
	printf("size = (%d)\n",size);
	printf("Please enter transmission granularity[1-10000]: \n"); 
	scanf("%d", &i);
	fprintf(output, "%d\n",i);  	
	for (i=0; i<size; i++)
	{
		num = rand() / 1000;
		fprintf(output, "%d\n",num);
	}
	fclose(output); 
} 
