// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef BASE_DEBUG_TRACE_EVENT_IMPL_H_
#define BASE_DEBUG_TRACE_EVENT_IMPL_H_
#pragma once

//#include "build/build_config.h"
#include <assert.h>
#include <algorithm>
#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>

//#include "event_trace_impl.h"
//#include "base/callback.h"
//#include "base/hash_tables.h"
//#include "base/memory/ref_counted_memory.h"
//#include "base/observer_list.h"
//#include "base/string_util.h"
//#include "base/synchronization/lock.h"
//#include "base/timer.h"

// Older style trace macros with explicit id and extra data
// Only these macros result in publishing data to ETW as currently implemented.
#define TRACE_EVENT_BEGIN_ETW(name, id, extra) \
    base::debug::TraceLog::AddTraceEventEtw( \
        TRACE_EVENT_PHASE_BEGIN, \
        name, reinterpret_cast<const void*>(id), extra)

#define TRACE_EVENT_END_ETW(name, id, extra) \
    base::debug::TraceLog::AddTraceEventEtw( \
        TRACE_EVENT_PHASE_END, \
        name, reinterpret_cast<const void*>(id), extra)

#define TRACE_EVENT_INSTANT_ETW(name, id, extra) \
    base::debug::TraceLog::AddTraceEventEtw( \
        TRACE_EVENT_PHASE_INSTANT, \
        name, reinterpret_cast<const void*>(id), extra)

template <typename Type>
struct StaticMemorySingletonTraits;

namespace event_trace
{

const int kTraceMaxNumArgs = 2;

// Output records are "Events" and can be obtained via the
// OutputCallback whenever the tracing system decides to flush. This
// can happen at any time, on any thread, or you can programatically
// force it to happen.
class  TraceEvent {
 public:
  union TraceValue {
    bool as_bool;
    unsigned long long as_uint;
    long long as_int;
    double as_double;
    const void* as_pointer;
    const char* as_string;
  };

  TraceEvent();
  TraceEvent(int thread_id,
             event_trace::TimeTicks timestamp,
             char phase,
             const unsigned char* category_enabled,
             const char* name,
             unsigned long long id,
             int num_args,
             const char** arg_names,
             const unsigned char* arg_types,
             const unsigned long long* arg_values,
             unsigned char flags);
  ~TraceEvent();

  // Serialize event data to JSON
  static void AppendEventsAsJSON(const std::vector<TraceEvent>& events,
                                 size_t start,
                                 size_t count,
                                 std::string* out);
  void AppendAsJSON(std::string* out) const;

  event_trace::TimeTicks timestamp() const { return timestamp_; }


  const char* name() const { return name_; }

 private:
  // Note: these are ordered by size (largest first) for optimal packing.
  event_trace::TimeTicks timestamp_;
  // id_ can be used to store phase-specific data.
  unsigned long long id_;
  TraceValue arg_values_[kTraceMaxNumArgs];
  const char* arg_names_[kTraceMaxNumArgs];
  const unsigned char* category_enabled_;
  const char* name_;
  char* parameter_copy_storage_;
  int thread_id_;
  char phase_;
  unsigned char flags_;
  unsigned char arg_types_[kTraceMaxNumArgs];
};





class  TraceLog {
private:
	static TraceLog gStaticTraceInstance;
 public:
	 static TraceLog* GetInstance() { return &gStaticTraceInstance;} ;

  // Get set of known categories. This can change as new code paths are reached.
  // The known categories are inserted into |categories|.
  void GetKnownCategories(std::vector<std::string>* categories);

  // Enable tracing for provided list of categories. If tracing is already
  // enabled, this method does nothing -- changing categories during trace is
  // not supported.
  // If both included_categories and excluded_categories are empty,
  //   all categories are traced.
  // Else if included_categories is non-empty, only those are traced.
  // Else if excluded_categories is non-empty, everything but those are traced.
  // Wildcards * and ? are supported (see MatchPattern in string_util.h).
  void SetEnabled(const std::vector<std::string>& included_categories,
                  const std::vector<std::string>& excluded_categories);

  // |categories| is a comma-delimited list of category wildcards.
  // A category can have an optional '-' prefix to make it an excluded category.
  // All the same rules apply above, so for example, having both included and
  // excluded categories in the same list would not be supported.
  //
  // Example: SetEnabled("test_MyTest*");
  // Example: SetEnabled("test_MyTest*,test_OtherStuff");
  // Example: SetEnabled("-excluded_category1,-excluded_category2");
  void SetEnabled(const std::string& categories);

  // Retieves the categories set via a prior call to SetEnabled(). Only
  // meaningful if |IsEnabled()| is true.
  void GetEnabledTraceCategories(std::vector<std::string>* included_out,
                                 std::vector<std::string>* excluded_out);

  // Disable tracing for all categories.
  void SetDisabled();
  // Helper method to enable/disable tracing for all categories.
  void SetEnabled(bool enabled);
  bool IsEnabled() { return enabled_; }

  // Enabled state listeners give a callback when tracing is enabled or
  // disabled. This can be used to tie into other library's tracing systems
  // on-demand.
  class EnabledStateChangedObserver {
   public:
    // Called just before the tracing system becomes
    // enabled. TraceLog::IsEnabled will return false at this point and trace
    // macros and methods called within the observer will deadlock.
    virtual void OnTraceLogWillEnable() { }

    // Called just before the tracing system disables. TraceLog::IsEnabled is
    // still false at this point TRACE macros will still be capturing
    // data. However, trace macros and methods called within the observer will
    // deadlock.
    virtual void OnTraceLogWillDisable() { }
  };
  void AddEnabledStateObserver(EnabledStateChangedObserver* listener);
  void RemoveEnabledStateObserver(EnabledStateChangedObserver* listener);

  float GetBufferPercentFull() const;

 

  // Flushes all logged data to the callback.
  void Flush();

  // simply annotates the stream with "[" and "]" respectivly
  void BeginLogging();
  void EndLogging();

  // Called by TRACE_EVENT* macros, don't call this directly.
  static const unsigned char* GetCategoryEnabled(const char* name);
  static const char* GetCategoryName(const unsigned char* category_enabled);

  // Called by TRACE_EVENT* macros, don't call this directly.
  // Returns the index in the internal vector of the event if it was added, or
  //         -1 if the event was not added.
  // On end events, the return value of the begin event can be specified along
  // with a threshold in microseconds. If the elapsed time between begin and end
  // is less than the threshold, the begin/end event pair is dropped.
  // If |copy| is set, |name|, |arg_name1| and |arg_name2| will be deep copied
  // into the event; see "Memory scoping note" and TRACE_EVENT_COPY_XXX above.
  int AddTraceEvent(char phase,
                    const unsigned char* category_enabled,
                    const char* name,
                    unsigned long long id,
                    int num_args,
                    const char** arg_names,
                    const unsigned char* arg_types,
                    const unsigned long long* arg_values,
                    int threshold_begin_id,
                    long long threshold,
                    unsigned char flags);
  static void AddTraceEventEtw(char phase,
                               const char* name,
                               const void* id,
                               const char* extra);
  static void AddTraceEventEtw(char phase,
                               const char* name,
                               const void* id,
                               const std::string& extra);

  int process_id() const { return process_id_; }

  // Exposed for unittesting:

  // Allow tests to inspect TraceEvents.
  size_t GetEventsSize() const { return logged_events_.size(); }
  const TraceEvent& GetEventAt(size_t index) const {
    assert(index < logged_events_.size());
    return logged_events_[index];
  }

  void SetProcessID(int process_id);

 private:
  // This allows constructor and destructor to be private and usable only
  // by the Singleton class.
  friend struct StaticMemorySingletonTraits<TraceLog>;

  TraceLog();
  ~TraceLog();
  const unsigned char* GetCategoryEnabledInternal(const char* name);
  void AddThreadNameMetadataEvents();
  void AddClockSyncMetadataEvents();

  // TODO(nduca): switch to per-thread trace buffers to reduce thread
  // synchronization.
  Lock lock_;
  bool enabled_;
  std::vector<TraceEvent> logged_events_;
  std::vector<std::string> included_categories_;
  std::vector<std::string> excluded_categories_;

  std::map<int, std::string> thread_names_;

  // XORed with TraceID to make it unlikely to collide with other processes.
  unsigned long long process_id_hash_;

  int process_id_;

  //DISALLOW_COPY_AND_ASSIGN(TraceLog);
};

}
#endif  // BASE_DEBUG_TRACE_EVENT_IMPL_H_
