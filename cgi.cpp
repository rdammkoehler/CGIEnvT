#include <stdio.h>
#include <string.h>
#include "cgiEnvT.hpp"

int main(int argc, char *argv[], char *envp[])
{
	cgiEnvT *CGI = new cgiEnvT(argc,argv,envp);

	printf("<pre>%s</pre>\n",CGI->toString());

	//while(getchar() != '\n');

	return(0);
}
