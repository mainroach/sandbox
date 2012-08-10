
#include <windows.h>
#include <process.h>

#include <stdio.h>
#include <assert.h>
#include <map>


#include "../event_trace.h"
#include "../event_trace_plat.h"

FILE* logfil = NULL;

namespace event_trace
{
	
   void OutputCallback (const char* pData) { printf(pData); fwrite(pData,strlen(pData),1,logfil);  }
   void BufferFullCallback() {TRACE_EVENT_FLUSH_LOG(); }
};

#define BASIC_TEST
#define THREADED_TEST

#ifdef THREADED_TEST
void workFunction(void* pData)
{

	{
      
		TRACE_EVENT0("MY_SUBSYSTEM", "doSomethingCostly");

		for(int i =0; i < 1000000; i++)
		{
			const int c = 20+ 30 + rand()%500 - GetCurrentThreadId();
         char* pdat = new char[1];
         delete[] pdat;
		}
      
	}

   TRACE_EVENT_INSTANT0("MY_SUBSYSTEM", "EVENT HIT THE FAN");
   {
		TRACE_EVENT0("MY_SUBSYSTEM", "doSomethingCostly222");
		for(int i =0; i < 1000000; i++)
		{
			const int c = 20+ 30 + rand()%500 - GetCurrentThreadId();
         char* pdat = new char[1];
         delete[] pdat;
         
		}
	}


    {
		TRACE_EVENT0("MY_SUBSYSTEM", "doSomethingCostly333");
		for(int i =0; i < 1000; i++)
		{
			TRACE_EVENT2("MY_SUBSYSTEM", "ILIKECHEESE","objName","Falcore","HPVal","88");
			const int c = 20+ 30 + rand()%500 - GetCurrentThreadId();
         char* pdat = new char[1];
         delete[] pdat;
         
		}
	}

}

void workFunction2(void* pData)
{

	{
		TRACE_EVENT0("MY_SUBSYSTEM", "22doSomethingCostly");
		for(int i =0; i < 10000; i++)
		{
			const int c = 20+ 30 + rand()%500 - GetCurrentThreadId();
         char* pdat = new char[1];
         delete[] pdat;
         
		}
	}

   TRACE_EVENT_INSTANT0("MY_SUBSYSTEM", "EVENT HIT THE FAN 2");
   {
		TRACE_EVENT0("MY_SUBSYSTEM", "22doSomethingCostly222");
		for(int i =0; i < 10000; i++)
		{
			const int c = 20+ 30 + rand()%500 - GetCurrentThreadId();
         char* pdat = new char[1];
         delete[] pdat;
         
		}
	}


    {
		TRACE_EVENT0("MY_SUBSYSTEM", "22doSomethingCostly333");
		for(int i =0; i < 10000; i++)
		{
			const int c = 20+ 30 + rand()%500 - GetCurrentThreadId();
         char* pdat = new char[1];
         delete[] pdat;
         
		}
	}

}


#endif
int main()
{
	logfil = fopen("out.json","wb");

	
	TRACE_EVENT_BEGIN();
   


#ifdef BASIC_TEST
   workFunction(0);
#endif


#ifdef THREADED_TEST
   	uintptr_t threadHandle = _beginthread(workFunction2, 0, 0);
      workFunction(0);
#endif

      
		TRACE_EVENT_FINISH();
      fclose(logfil);
      logfil = 0;

}