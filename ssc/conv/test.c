#include <string.h>
#include <locale.h>
#include <stdio.h>
#define LENGTH 40
int counter = 0;
int chn()
{
	if ( counter < 100 ) printf("%d\n",counter);
	else return -1;
	return 0;
}
main()
{
 char string[40];
 char ptr[5],i=0;
	while ( !chn() )
		counter++;
}
changer()
{
 char	string1[LENGTH], delimiters[LENGTH];
 char	*pstr;
 int	counter;

	(void)setlocale(LC_ALL, "");
	printf("Enter the string to be searched: ");
	sprintf(delimiters," \t\n");
	if ( fgets(string1, LENGTH, stdin) != NULL ) {
		if ( (pstr = strtok(string1, delimiters)) != NULL )
			printf("String1 is %s \n",string1);
			printf("Token 1 is %s\n",pstr);
			counter = 2;
			while ((pstr = strtok((char *)NULL, delimiters)) != NULL) {
				printf("Token %d is %s\n",counter, pstr);
				counter++;
			}
	}
}

