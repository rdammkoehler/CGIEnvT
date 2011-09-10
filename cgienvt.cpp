#include "cgiEnvT.hpp"

int length(char *p,char term);

cgiEnvT::cgiEnvT()
{	
	ArgSize = 0;
	ArgVars = NULL;

	cgiIn = stdin;
	cgiOut = stdout;
	cgiError = stderr;
	cgiDebug = stderr;
	cgiLog = stderr;

	EnvSize = 0;
	EnvVars = NULL;

	DataSize = 0;
	DataVars = NULL;

	return;
};

cgiEnvT::cgiEnvT(int argc, char *argv[], char *envp[])
{	
	int errcd = 0;
	long size = 0;
	int i = 0, j, offset;
	char *p = envp[i], *ContentLength;;
					   
	ArgSize = argc;
	ArgVars = argv;

	cgiOut = stdout;
	cgiError = stderr;
	cgiDebug = stderr;
	cgiLog = stderr;

	EnvSize = 0;
	
	while(p != NULL) p = envp[++EnvSize];

	EnvVars = (struct STRTUPLE *) malloc(sizeof(struct STRTUPLE) * EnvSize); 

	for (i = 0; envp[i] != NULL; i++)
	{
		p = envp[i];
		EnvVars[i].name = (char *) malloc(sizeof(char) * length(p,'=') + 1);
		for(offset = j = 0; j < (int) strlen(p) && p[j] != '=' && j-offset < STRMAX;j++)
			EnvVars[i].name[j-offset] = p[j];
		EnvVars[i].name[j-offset] = '\0'; //<<ensure string termination
		EnvVars[i].value = (char *) malloc(sizeof(char) * length(p+j+1,'\n') + 1);
		for(offset = ++j; 	j < (int) strlen(p) && p[j] != '\n' && j-offset < STRMAX;j++)
			EnvVars[i].value[j-offset] = p[j];
		EnvVars[i].value[j-offset] = '\0';
		size++;
	}

	
	if ((ArgSize > 1) && (strcmp((NULL == GetNthArgValue(1))?GetNthArgValue(0):GetNthArgValue(1),"-f") == 0))	//look at arg[1] for a -f, if it is, then arg[2] is a filename to readfrom
	{
		if (GetNthArgValue(2) != NULL)
		{
			if ((cgiIn = fopen(GetNthArgValue(2),"r")) == NULL) 
			{
				ReportError("%s Failed to open %s for input. File Not Accessable.\n",GetNthArgValue(0),GetNthArgValue(2));
				errcd = 1;
			}
			else
			{	//now that we have it open we need to set CONTENT_LENGTH to the file size
				if (atol(((ContentLength = GetEnvValue("CONTENT_LENGTH")) != NULL)?ContentLength:"0") == 0)
				{
					EnvVars = (struct STRTUPLE *) realloc(EnvVars,sizeof(struct STRTUPLE) * ++EnvSize);
					EnvVars[EnvSize-1].name = (char *) malloc(sizeof(char) * 15);
					strcpy(EnvVars[EnvSize-1].name,"CONTENT_LENGTH");
					EnvVars[EnvSize-1].value = (char *) malloc(sizeof(char) * 5);
					sprintf(EnvVars[EnvSize-1].value,"%d",STRMAX);
				}
			}
		}
		else
			ReportError("%s Failed to open file. File Name wasn't Provided.\n",GetNthArgValue(0));
	}
	else
		cgiIn = stdin;

	//if successfull
	if (errcd == 0)
	{
		//must determine where data comes from here!
		DataSize = LoadData(); 
		CgiOut("Content-type: text/html\n\n");
	}
	else
	{
		CgiOut("Content Type: text/html\n\n<html><title>CGI Error</title>\n<body>\n<h1>CGI Error</h1>\nAn error occured while creating a response.\n</body></html>");
		ReportError("Error Code %d occured while creating a response.\n",errcd);
	}

	return;
};

char *cgiEnvT::GetEnvValue(char *VarName)
{
	long i = 0;	

	for(i = 0; (i < EnvSize) && (strcmp(EnvVars[i].name,VarName) != 0);i++);
/*#ifndef _UNIX
	for(i = 0; (strcmp(EnvVars[i].name,VarName) != 0) && (i < EnvSize) ;i++);
#else
    int cmpres = 2;
	//UNIX GCC is so crappy that we need to have cmpres be a separate value, inline this bombs on UNIX
    for(i = 0; (cmpres != 0) && (i < EnvSize) ;i++)
            cmpres = strcmp(EnvVars[i].name, VarName);
#endif	*/
	return((i<EnvSize)?EnvVars[i].value:NULL);
}

int cgiEnvT::LoadData(char *inputfile)
{
	long ict = 0;

	if (0 == SetCgiIn(inputfile))
	{	
		long i=0, j=0, offset=0;
		char *p, *q = (char *) malloc(sizeof(char) * STRMAX);
		char *input;
		char *xtra = (char *) malloc(sizeof(char) * MAXCGIINPUTLENGTH);

		input = (char *) malloc(sizeof(char) * MAXCGIINPUTLENGTH);
		fgets(input,MAXCGIINPUTLENGTH,cgiIn);
		while(!feof(cgiIn))
                {
			fgets(xtra,MAXCGIINPUTLENGTH,cgiIn);
			strcat(input,xtra);
		}
		free(xtra);

		ict = ((int) strlen(input) > 0)?1:0;
		for(j = 0; j < (int) strlen(input); ict += (input[j++] == '&')?1:0);
		if ((ict != 0) && (q != NULL))
		{
			if (DataSize == 0)
				DataVars = (struct STRTUPLE *) malloc(sizeof(struct STRTUPLE) * ict);
			else
				DataVars = (struct STRTUPLE *) realloc(DataVars,sizeof(struct STRTUPLE) * (DataSize + ict));
			strcpy(q,input);
			p = q;
			for(i = 0; i < ict; i++)
			{
				DataVars[i].name = (char *) malloc(sizeof(char) * length(p,'=') + 1);
				for(offset = j = 0; j < (int) strlen(p) && p[j] != '=' && j-offset < STRMAX; j++)
					DataVars[i].name[j-offset] = p[j];
				DataVars[i].name[j-offset] = '\0';
				DataVars[i].value = (char *) malloc(sizeof(char) * length(p+j+1,'&') + 1);
				for(offset = ++j; 	j < (int) strlen(p) && p[j] != '&' && j-offset < STRMAX; j++)
					DataVars[i].value[j-offset] = p[j];
				DataVars[i].value[j-offset] = '\0';
				p += ++j;
			}
		}
		free(input);	
		free(q);
	}
	else
	{
		ict = 0;
	}

	DataSize += ict;

	return(ict);
}

int cgiEnvT::LoadData()
{
// if method is GET or HEAD, data is going to be in QUERY_STRING
// if method is POST, data is going to be on cgiIn (a.k.a. STDIN)
// if method is not in the ENV or == "" then use QUERY_STRING and the Command Line

	long ict = 0, i, j, k = 0, offset, errcd = 0;
	char *p, *q = (char *) malloc(sizeof(char) * STRMAX), *ContentLength, *input, *CgiMethod, *tmpstr;

	if (atol(((ContentLength = GetEnvValue("CONTENT_LENGTH")) != NULL)?ContentLength:"0") > 0)
	{
		CgiMethod = ((tmpstr = GetEnvValue("REQUEST_METHOD")) != NULL)?tmpstr:(char *)"NO_POST_METHOD";
		if ((strcmp(CgiMethod,"POST") == 0) || (strcmp((NULL == GetNthArgValue(1))?GetNthArgValue(0):GetNthArgValue(1),"-f") == 0))
		{
			char *xtra = (char *) malloc(sizeof(char) * MAXCGIINPUTLENGTH);

			input = (char *) malloc(sizeof(char) * MAXCGIINPUTLENGTH);
			fgets(input,MAXCGIINPUTLENGTH,cgiIn);
			while(!feof(cgiIn))
			{
				fgets(xtra,MAXCGIINPUTLENGTH,cgiIn);
				strcat(input,xtra);
			}
			free(xtra);
		}
		else
		{
			input = (char *) malloc(sizeof(char) * STRMAX);
			if ((strcmp(CgiMethod,"GET") == 0) || (strcmp(CgiMethod,"HEAD") == 0))
				input = GetEnvValue("QUERY_STRING");
			else
			{
				input = GetEnvValue("QUERY_STRING");
				//add all of argv here
				for ( i = 0; i < ArgSize; i++)
				{
					input = (char *) realloc(input, sizeof(char) * ((int) strlen(ArgVars[i])+1));
					strcat(input,"&");
					strcat(input,ArgVars[i]); //are the command line argv's in the form name=value?
				}
			}
		}
	}
	else
	{	//prompt user for input here, if appropriate
		if (strcmp((NULL == GetNthArgValue(1))?GetNthArgValue(0):GetNthArgValue(1),"-f") != 0)	//look at arg[1] for a -f, if its there, we read from an alternate file	
		{
			fprintf(stdout,"Enter Your Input to %s:\n",GetNthArgValue(0));
			input = (char *) malloc(sizeof(char) * MAXCGIINPUTLENGTH);
			fgets(input,MAXCGIINPUTLENGTH,stdin);
		}
		else
			errcd = 32;	//same as ReportError("%s Failed to open file. File Name wasn't Provided.\n",GetNthArgValue(0));
	
	}

	if (errcd == 0)
	{
		ict = ((int) strlen(input) > 0)?1:0;
		for(k = 0; k < (int) strlen(input); ict += (input[k++] == '&')?1:0);
		if ((ict != 0) && (q != NULL))
		{
			DataVars = (struct STRTUPLE *) malloc(sizeof(struct STRTUPLE) * ict);
			strcpy(q,input);
			p = q;
			for(i = 0; i < ict; i++)
			{
				DataVars[i].name = (char *) malloc(sizeof(char) * length(p,'=') + 1);
				for(offset = j = 0; j < (int) strlen(p) && p[j] != '=' && j-offset < STRMAX; j++)
					DataVars[i].name[j-offset] = p[j];
				DataVars[i].name[j-offset] = '\0';
				DataVars[i].value = (char *) malloc(sizeof(char) * length(p+j+1,'&') + 1);
				for(offset = ++j; 	j < (int) strlen(p) && p[j] != '&' && j-offset < STRMAX; j++)
					DataVars[i].value[j-offset] = p[j];
				DataVars[i].value[j-offset] = '\0';
				p += ++j;
			}
		}
		free(input);
	}

	free(q);

	return (ict);
}

int length(char *p,char term)
{
	int i = 0;
	for(;i < STRMAX && i < (int) strlen(p) && p[i] != term && p[i] != '\n' && p[i] != '\0'; i++);

	return(i);
}

char *cgiEnvT::toString()
{
	char *p = (char *) malloc(sizeof(char) * ((EnvSize * sizeof(STRTUPLE))+(DataSize * sizeof(STRTUPLE))+(ArgSize * STRMAX)+1024));
	long i;

	if (p == NULL)
		ReportError("Not Enough Memory to create *p in toString, needed %d bytes.\n",((EnvSize * sizeof(STRTUPLE))+(DataSize * sizeof(STRTUPLE))+(ArgSize * STRMAX)+1024));
	else
	{
		sprintf(p,"cgiEnvT:\n");

		sprintf(p,"%sEnvVars: (%ld)\n",p,EnvSize);
		for(i = 0; i < EnvSize; i++)
			sprintf(p,"%sName: %s\nValue: %s\n\n",p,EnvVars[i].name,EnvVars[i].value);
		sprintf(p,"%s\n",p);

		sprintf(p,"%sDataVars (%ld)\n",p,DataSize);
		for(i = 0; i < DataSize; i++)
			sprintf(p,"%sName: %s\nValue: %s\n\n",p,DataVars[i].name,DataVars[i].value);
		sprintf(p,"%s\n",p);

		sprintf(p,"%sArguments: (%ld)\n",p,ArgSize);
		for(i = 0; i < ArgSize; i++)
			sprintf(p,"%sArgument[%d]: %s\n",p,i,ArgVars[i]);
		sprintf(p,"%s\n",p);
	}
	
	return(p);
}

void cgiEnvT::ReportError(char *fmt, ...) //only handles %d, %f, and %s
{
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;
	
	fprintf(cgiError,"Error: ");
	va_start(ap,fmt);
	for (p = fmt; *p ; p++)
	{
		if (*p != '%')
		{
			putc(*p,cgiError);
			continue;
		}
		switch (*++p) 
		{
			case 'd':
				ival = va_arg(ap,long);
				fprintf(cgiError,"%ld",ival);
				break;
			case 'f':
				dval = va_arg(ap,double);
				fprintf(cgiError,"%f",dval);
				break;
			case 's':
				for (sval = va_arg(ap,char *); *sval; sval++)
					putc(*sval,cgiError);
				break;
			default :
				putc(*p,cgiError);
				break;
		}
	}
	va_end(ap);
	
	return;
}

void cgiEnvT::CgiOut(char *fmt, ...)
{
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;
	
	va_start(ap,fmt);
	for (p = fmt; *p ; p++)
	{
		if (*p != '%')
		{
			putc(*p,cgiOut);
			continue;
		}
		switch (*++p) 
		{
			case 'd':
				ival = va_arg(ap,long);
				fprintf(cgiOut,"%ld",ival);
				break;
			case 'f':
				dval = va_arg(ap,double);
				fprintf(cgiOut,"%f",dval);
				break;
			case 's':
				for (sval = va_arg(ap,char *); *sval; sval++)
					putc(*sval,cgiOut);
				break;
			default :
				putc(*p,cgiOut);
				break;
		}
	}
	va_end(ap);
	
	return;
}

void cgiEnvT::CgiComment(char *fmt, ...)
{	
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;

	fputs("<!--",cgiOut);
	
	va_start(ap,fmt);
	for (p = fmt; *p ; p++)
	{
		if (*p != '%')
		{
			putc(*p,cgiOut);
			continue;
		}
		switch (*++p) 
		{
			case 'd':
				ival = va_arg(ap,long);
				fprintf(cgiOut,"%ld",ival);
				break;
			case 'f':
				dval = va_arg(ap,double);
				fprintf(cgiOut,"%f",dval);
				break;
			case 's':
				for (sval = va_arg(ap,char *); *sval; sval++)
					putc(*sval,cgiOut);
				break;
			default :
				putc(*p,cgiOut);
				break;
		}
	}
	va_end(ap);
	
	fputs("-->\n",cgiOut);

	return;
}

void cgiEnvT::CgiLog(char *fmt, ...)
{
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;

#ifndef _UNIX
	char *time = (char *) malloc(sizeof(char) * 50);
	fprintf(cgiLog,"%s Log: ",_strtime(time));
	free(time);
#else
	fprintf(cgiLog,"Log: ");
#endif

	va_start(ap,fmt);
	for (p = fmt; *p ; p++)
	{
		if (*p != '%')
		{
			putc(*p,cgiLog);
			continue;
		}
		switch (*++p) 
		{
			case 'd':
				ival = va_arg(ap,long);
				fprintf(cgiLog,"%ld",ival);
				break;
			case 'f':
				dval = va_arg(ap,double);
				fprintf(cgiLog,"%f",dval);
				break;
			case 's':
				for (sval = va_arg(ap,char *); *sval; sval++)
					putc(*sval,cgiLog);
				break;
			default :
				putc(*p,cgiLog);
				break;
		}
	}
	va_end(ap);
	
	return;
}

void cgiEnvT::CgiDebug(char *fmt, ...)
{
	va_list ap;
	char *p, *sval;
	int ival;
	double dval;

	fprintf(cgiDebug,"Debug: ");
	
	va_start(ap,fmt);
	for (p = fmt; *p ; p++)
	{
		if (*p != '%')
		{
			putc(*p,cgiDebug);
			continue;
		}
		switch (*++p) 
		{
			case 'd':
				ival = va_arg(ap,long);
				fprintf(cgiDebug,"%ld",ival);
				break;
			case 'f':
				dval = va_arg(ap,double);
				fprintf(cgiDebug,"%f",dval);
				break;
			case 's':
				for (sval = va_arg(ap,char *); *sval; sval++)
					putc(*sval,cgiDebug);
				break;
			default :
				putc(*p,cgiDebug);
				break;
		}
	}
	va_end(ap);
	
	return;
}
