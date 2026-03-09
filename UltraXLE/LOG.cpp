#include "ultra.h"
#include "stdsdk.h"

void printtxt(char *txt)
{/*
    static FILE *logfile;
    char *p;
    static int printedstuff=0;
    static int flushcnt=0;

    if(init.showconsole)
    {
		if(!logfile) logfile=fopen("T:\\Misc\\ultra.log","wt");
        if(!txt)
        {
            if(flushcnt)
            {
                flushcnt=0;
                fflush(logfile);
            }
            return;
        }

        flushcnt++;

        //view_writeconsole(txt);

        if(flushcnt>1000)
        {
            // redraw screen every 1000 messages
            //flushdisplay();
        }
    }
    else
    {
        if(!txt) return;
    }

    p=txt;
    while(*p)
    {
        if(*p==0x01) p+=2;
        else if(*p==0x02) p+=2;
        else break;
    }

    if(init.showconsole)
    {
        fputs(p,logfile);
    }

    outputhook(p,txt); // for new ui*/
}

void printMemStatus()
{
	/*MEMORYSTATUS stat;
	CHAR memStat[512];
	CHAR temp[100];

	memStat[0] = '\0';
	
	GlobalMemoryStatus(&stat);
	
    sprintf(temp, "%4d total MB of virtual memory.\n", stat.dwTotalVirtual / (1024*1024) );
	strcat(memStat, temp);
    sprintf(temp, "%4d  free MB of virtual memory.\n", stat.dwAvailVirtual / (1024*1024) );
	strcat(memStat, temp);
    sprintf(temp, "%4d total MB of physical memory.\n", stat.dwTotalPhys / (1024*1024) );
    strcat(memStat, temp);
	sprintf(temp, "%4d  free MB of physical memory.\n", stat.dwAvailPhys / (1024*1024) );
	strcat(memStat, temp);
    sprintf(temp, "%4d total MB of paging file.\n", stat.dwTotalPageFile / (1024*1024) );
    strcat(memStat, temp);
	sprintf(temp, "%4d  free MB of paging file.\n", stat.dwAvailPageFile / (1024*1024) );
    strcat(memStat, temp);
	sprintf(temp, "%4d  percent of memory is in use.\n", stat.dwMemoryLoad );

	print(memStat);*/
}

void print(char *txt,...) // generic
{
    /*static HANDLE logfile = NULL;
    static char buf[512];
    va_list argp;
	DWORD numberOfBytesWritten;

    va_start(argp,txt);

	if(!logfile)
		logfile = CreateFile("T:\\Misc\\print.log", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
    //if(!logfile) logfile=fopen("print.log","wt");
    if(!txt)
    {
        //fflush(logfile);
        return;
    }

    vsprintf(buf,txt,argp);

	strcat(buf, "\n");

    //fputs(p,logfile);

	WriteFile(logfile, buf, strlen(buf), &numberOfBytesWritten, NULL);
	
	/*static char buf[256];
    va_list argp;

    va_start(argp,txt);

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    vsprintf(buf,txt,argp);
    printtxt(buf);*/
}

/*void logd(char *txt,...) // [d]isplay
{
	static FILE *logfile;
    static char buf[256];
    va_list argp;
    char *p;

    if(!st.dumpgfx) return;

    va_start(argp,txt);

    if(!logfile) logfile=fopen("dlist.log","wt");
    if(!txt)
    {
        fflush(logfile);
        return;
    }

    vsprintf(buf,txt,argp);
    p=buf;
    fputs(p,logfile);
}

void loga(char *txt,...) // [a]udio
{
    static FILE *logfile;
    static char buf[256];
    va_list argp;
    char *p;

    if(!st.dumpsnd) return;

    va_start(argp,txt);

    if(!logfile) logfile=fopen("slist.log","wt");
    if(!txt)
    {
        fflush(logfile);
        return;
    }

    vsprintf(buf,txt,argp);

    p=buf;
    fputs(p,logfile);
}*/

void logx(char *txt,...) // [a]udio
{
    /*static FILE *logfile;
    static char buf[256];
    va_list argp;
    char *p;

    va_start(argp,txt);

	if(!logfile) logfile=fopen("T:\\Misc\\xx.log","wt");
    if(!txt)
    {
        fflush(logfile);
        return;
    }

    vsprintf(buf,txt,argp);

    p=buf;
    fputs(p,logfile);*/
}

/*void logo(char *txt,...) // [o]s
{
    static char buf[256];
    va_list argp;

    if(!st.dumpos) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);

    vsprintf(buf,txt,argp);

    printtxt(buf);
}

void logh(char *txt,...) // [h]w
{
    static char buf[256];
    va_list argp;

    if(!st.dumphw) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);

    vsprintf(buf,txt,argp);

    printtxt(buf);
}

void logc(char *txt,...) // [c]ompiler
{
    static char buf[256];
    va_list argp;

    if(!st.dumpasm) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);

    vsprintf(buf,txt,argp);

    printtxt(buf);
}

// info logging
void logi(char *txt,...)
{
    static char buf[256];
    va_list argp;

    if(!st.dumpinfo) return;

    if(!txt)
    {
        printtxt(NULL);
        return;
    }

    va_start(argp,txt);
    vsprintf(buf,txt,argp);
    printtxt(buf);
}*/

void flushlog(void)
{/*
    loga(NULL);
    logd(NULL);
    printtxt(NULL);*/
}

// errors/warnings

void xception(char *txt,...)
{
    /*static char buf[256];
    va_list argp;

    st2.xception=1;
    va_start(argp,txt);
    sprintf(buf,YEL"xception(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
    if(1)
    {
        print(buf);
        cpu_break();
    }*/
}

void error(char *txt,...)
{/*
    static char buf[256];
    va_list argp;
    va_start(argp,txt);
    sprintf(buf,YEL"error(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
    if(st.stoperror)
    {
        print(buf);
        cpu_break();
    }
    else
    {
        print(buf);
    }*/
	    static char buf[256];
    va_list argp;
    va_start(argp,txt);
    sprintf(buf,YEL"error(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
	OutputDebugString(buf);
}

void warning(char *txt,...)
{/*
    static char buf[256];
    va_list argp;
    va_start(argp,txt);
    sprintf(buf,YEL"warning(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
    if(st.stopwarning)
    {
        print(buf);
        cpu_break();
    }
    else
    {
        print(buf);
    }*/
    static char buf[256];
    va_list argp;
    va_start(argp,txt);
    sprintf(buf,YEL"warning(%08X): ",st.pc);
    vsprintf(buf+strlen(buf),txt,argp);
    strcat(buf,"\n");
	OutputDebugString(buf);
}

