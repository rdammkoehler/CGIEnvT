#ifndef _CGIENVT
#define _CGIENVT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef _UNIX
	#include <time.h>
#endif

#define MAXCGIINPUTLENGTH 131072
#define STRMAX 8192

/*struct STRTUPLE {
	char name[STRMAX];
	char value[STRMAX];
};*/
struct STRTUPLE {
	char *name;
	char *value;
};

class cgiEnvT {
	public:
			//environment
			long GetEnvSize() { return(EnvSize); };
			char *GetEnvValue(char *VarName);
			char *GetNthEnvName(long n) { return((n <= EnvSize)?EnvVars[n].name:NULL); };
			char *GetNthEnvValue(long n) { return((n <= EnvSize)?EnvVars[n].value:NULL); };
			//data
			long GetDataSize() { return(DataSize); };
			char *GetDataValue(char *VarName);
			char *GetNthDataName(long n) { return((n <= DataSize)?DataVars[n].name:NULL); };
			char *GetNthDataValue(long n) { return((n <= DataSize)?DataVars[n].value:NULL); };
			int LoadData(char *inputfile);
			//arguments
			long GetArgSize() { return(ArgSize); };
			char *GetNthArgValue(int n) {  return((n <= ArgSize)?ArgVars[n]:NULL); };
			//output
			void CgiOut(char *fmt, ...);
			void CgiComment(char *fmt, ...); 
			void CgiLog(char *fmt, ...); 
			void CgiDebug(char *fmt, ...);
			char *toString();
			//files
			int SetCgiIn(char *in) { return(((cgiIn = fopen(in,"r"))!=NULL)?0:1); }; 
			int SetCgiOut(char *out) { return(((cgiOut = fopen(out,"w"))!=NULL)?0:1); };
			int SetCgiError(char *error) { return(((cgiError = fopen(error,"w"))!=NULL)?0:1); };
			int SetCgiDebug(char *debug) { return(((cgiDebug = fopen(debug,"w"))!=NULL)?0:1); };
			int SetCgiLog(char *log) { return(((cgiLog = fopen(log,"w"))!=NULL)?0:1); };
			
	//constructors
			cgiEnvT();
			cgiEnvT(int argc, char *argv[], char *envp[]);
								
	//destructor
			~cgiEnvT() { fclose(cgiIn); fclose(cgiOut); fclose(cgiError); fclose(cgiDebug); fclose(cgiLog); free(EnvVars); free(DataVars); };
										 
	private:
			FILE *cgiIn;
			FILE *cgiOut;
			FILE *cgiError;
			FILE *cgiDebug;
			FILE *cgiLog;

			long EnvSize;
			struct STRTUPLE *EnvVars;

			long DataSize;
			struct STRTUPLE *DataVars;
			int LoadData();

			int ArgSize;
			char **ArgVars;

			void ReportError(char *fmt, ...); //only handles %d, %f, and %s
};

int cgiMain(cgiEnvT *CGI);

#endif

