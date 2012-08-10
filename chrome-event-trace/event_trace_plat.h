/*
	This file defines platform specific items that are required by event_trace
*/
#ifndef EVENT_TRACE_PLAT_H
#define EVENT_TRACE_PLAT_H


#include <time.h>
#ifdef OS_WIN
#include <windows.h>
#elif defined(__GNUC__)
#include <sys/time.h>
#include <pthread.h>
#endif

namespace event_trace
{



////////////////////////////////
//TYPEDEFS
////////////////////////////////
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;


#ifdef OS_WIN

typedef __int64 int64;
typedef unsigned __int64 uint64;
#define _tlsVal __declspec(thread)

#elif defined(__GNUC__)

typedef long long int64;
typedef unsigned long long uint64;
#define _tlsVal __thread

#endif


////////////////////////////////
//TIMERS
////////////////////////////////
typedef unsigned long long timer_ticks;
typedef  timer_ticks TimeTicks;
#if defined(OS_WIN) 
inline void query_counter(timer_ticks *pTicks){
   QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(pTicks));
}

inline void query_counter_frequency(timer_ticks *pTicks){
   QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(pTicks));
}
#elif defined(__GNUC__)
#include <sys/time.h>
inline void query_counter(timer_ticks *pTicks){
   struct timeval cur_time;
   gettimeofday(&cur_time, NULL);
   *pTicks = static_cast<unsigned long long>(cur_time.tv_sec)*1000000ULL + static_cast<unsigned long long>(cur_time.tv_usec);
}
inline void query_counter_frequency(timer_ticks *pTicks){
   *pTicks = 1000000;
}
#endif


////////////////////////////////
//LOCKING
////////////////////////////////
class Lock
{

#if defined(OS_WIN) 
public:
	Lock(){InitializeCriticalSectionAndSpinCount(&mCrit, 0); };
	~Lock(){DeleteCriticalSection(&mCrit); };
	void Aquire(){EnterCriticalSection(&mCrit);};
	void Release(){LeaveCriticalSection(&mCrit);};
	
private:
	CRITICAL_SECTION mCrit;
#elif defined(__GNUC__)

public:
	Lock(){pthread_mutex_init(&mCrit, 0); };
	~Lock(){pthread_mutex_destroy(&mCrit); };
	void Aquire(){pthread_mutex_lock(&mCrit);};
	void Release(){pthread_mutex_unlock(&mCrit);};

private:
	pthread_mutex_t mCrit;
#endif
};


class ScopedLock : public Lock
{
public:
	ScopedLock(Lock& lock):pLock_(&lock) {lock.Aquire();};
   ~ScopedLock(){ if(pLock_) pLock_->Release(); pLock_=0; };
private:
	Lock* pLock_;
};



//threading
inline uint32 getCurrentProcId()
{
#ifdef OS_WIN
	return GetCurrentProcessId();
#elif defined(__GNUC__)
	return getpid();
#endif
}

inline uint32 getCurrentThreadId()
{
#ifdef OS_WIN
	return GetCurrentThreadId();
#elif defined(__GNUC__)
	return pthread_self();
#endif
}



   // When enough events are collected, they are handed (in bulk) to
  // the output callback. If no callback is set, the output will be
  // silently dropped. The callback must be thread safe.
  //typedef RefCountedData<std::string> std::string*;
  //typedef base::Callback<void(std::string*)> OutputCallback;
  void OutputCallback(const char*);

  // The trace buffer does not flush dynamically, so when it fills up,
  // subsequent trace events will be dropped. This callback is generated when
  // the trace buffer is full. The callback must be thread safe.
  //typedef base::Callback<void(void)> BufferFullCallback;
  void BufferFullCallback();
}

#endif 