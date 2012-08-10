// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "event_trace.h"
#include "event_trace_plat.h"
#include "event_trace_impl.h"

#include <algorithm>
#include <string.h>
#include <assert.h>
/*
#include "base/bind.h"
#include "base/debug/trace_event.h"
#include "base/file_util.h"
#include "base/format_macros.h"
#include "base/lazy_instance.h"
#include "base/memory/singleton.h"
#include "base/process_util.h"
#include "base/stringprintf.h"
#include "base/string_tokenizer.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_local.h"
#include "base/utf_string_conversions.h"
#include "base/stl_util.h"
#include "base/sys_info.h"
#include "base/third_party/dynamic_annotations/dynamic_annotations.h"
#include "base/time.h"


*/



// Controls the number of trace events we will buffer in-memory
// before throwing them away.
const size_t kTraceEventBufferSize = 500000;
const size_t kTraceEventBatchSize = 1000;

#define TRACE_EVENT_MAX_CATEGORIES 100

namespace event_trace {

// Parallel arrays g_categories and g_category_enabled are separate so that
// a pointer to a member of g_category_enabled can be easily converted to an
// index into g_categories. This allows macros to deal only with char enabled
// pointers from g_category_enabled, and we can convert internally to determine
// the category name from the char enabled pointer.
const char* g_categories[TRACE_EVENT_MAX_CATEGORIES] = {
  "tracing already shutdown",
  "tracing categories exhausted; must increase TRACE_EVENT_MAX_CATEGORIES",
  "__metadata",
};
// The enabled flag is char instead of bool so that the API can be used from C.
unsigned char g_category_enabled[TRACE_EVENT_MAX_CATEGORIES] = { 0 };
const int g_category_already_shutdown = 0;
const int g_category_categories_exhausted = 1;
const int g_category_metadata = 2;
int g_category_index = 3; // skip initial 3 categories

// The most-recently captured name of the current thread
//LazyInstance<ThreadLocalPointer<const char> >::Leaky g_current_thread_name = LAZY_INSTANCE_INITIALIZER;
// Flag to indicate whether we captured the current thread name
static _tlsVal bool g_current_thread_name_captured;


void AppendValueAsJSON(unsigned char type,
                       TraceEvent::TraceValue value,
                       std::string* out) {
	char temp_string[128];
  std::string::size_type start_pos;
  switch (type) {
  case TRACE_VALUE_TYPE_BOOL:
      *out += value.as_bool ? "true" : "false";
      break;
    case TRACE_VALUE_TYPE_UINT:
      sprintf(temp_string,"%llu", static_cast<uint64>(value.as_uint));
		*out += temp_string;
      break;
    case TRACE_VALUE_TYPE_INT:
      sprintf(temp_string, "%lld", static_cast<int64>(value.as_int));
		*out += temp_string;
      break;
    case TRACE_VALUE_TYPE_DOUBLE:
     sprintf(temp_string, "%f", value.as_double);
		*out += temp_string;
      break;
    case TRACE_VALUE_TYPE_POINTER:
      // JSON only supports double and int numbers.
      // So as not to lose bits from a 64-bit pointer, output as a hex string.
      sprintf(temp_string, "\"%llu\"", static_cast<uint64>(
                                     reinterpret_cast<intptr_t>(
                                     value.as_pointer)));
		*out += temp_string;
      break;
    case TRACE_VALUE_TYPE_STRING:
    case TRACE_VALUE_TYPE_COPY_STRING:
      *out += "\"";
      start_pos = out->size();
      *out += value.as_string ? value.as_string : "NULL";
      // insert backslash before special characters for proper json format.
      while ((start_pos = out->find_first_of("\\\"", start_pos)) !=
             std::string::npos) {
        out->insert(start_pos, 1, '\\');
        // skip inserted escape character and following character.
        start_pos += 2;
      }
      *out += "\"";
      break;
    default:
      printf( "Don't know how to print this value");
      break;
  }
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
//
// TraceEvent
//
////////////////////////////////////////////////////////////////////////////////

namespace {

size_t GetAllocLength(const char* str) { return str ? strlen(str) + 1 : 0; }

// Copies |*member| into |*buffer|, sets |*member| to point to this new
// location, and then advances |*buffer| by the amount written.
void CopyTraceEventParameter(char** buffer,
                             const char** member,
                             const char* end) {
  if (*member) {
      const int len = end - *buffer;
	  if(len <0) return;

	  int written = 0;
	  for(written =0;written < len ;written++)
	  {
		  if(*member[written] == '\0') break;
		  *buffer[written] = *member[written];
	  }
	  written++;

//CLM size_t written = strlcpy(*buffer, *member, end - *buffer) + 1;
    assert(static_cast<int>(written) <= end - *buffer);
    *member = *buffer;
    *buffer += written;
  }
}

}  // namespace


event_trace::TraceEvent::TraceEvent()
    : id_(0u),
      category_enabled_(NULL),
      name_(NULL),
      thread_id_(0),
      phase_(TRACE_EVENT_PHASE_BEGIN),
      flags_(0) {
  arg_names_[0] = NULL;
  arg_names_[1] = NULL;
  memset(arg_values_, 0, sizeof(arg_values_));
}

event_trace::TraceEvent::TraceEvent(int thread_id,
                       event_trace::TimeTicks timestamp,
                       char phase,
                       const unsigned char* category_enabled,
                       const char* name,
                       unsigned long long id,
                       int num_args,
                       const char** arg_names,
                       const unsigned char* arg_types,
                       const unsigned long long* arg_values,
                       unsigned char flags)
    : timestamp_(timestamp),
      id_(id),
      category_enabled_(category_enabled),
      name_(name),
      thread_id_(thread_id),
      phase_(phase),
      flags_(flags) {
  // Clamp num_args since it may have been set by a third_party library.
  num_args = (num_args > kTraceMaxNumArgs) ? kTraceMaxNumArgs : num_args;
  int i = 0;
  for (; i < num_args; ++i) {
    arg_names_[i] = arg_names[i];
    arg_values_[i].as_uint = arg_values[i];
    arg_types_[i] = arg_types[i];
  }
  for (; i < kTraceMaxNumArgs; ++i) {
    arg_names_[i] = NULL;
    arg_values_[i].as_uint = 0u;
    arg_types_[i] = TRACE_VALUE_TYPE_UINT;
  }

  bool copy = !!(flags & TRACE_EVENT_FLAG_COPY);
  size_t alloc_size = 0;
  if (copy) {
    alloc_size += GetAllocLength(name);
    for (i = 0; i < num_args; ++i) {
      alloc_size += GetAllocLength(arg_names_[i]);
      if (arg_types_[i] == TRACE_VALUE_TYPE_STRING)
        arg_types_[i] = TRACE_VALUE_TYPE_COPY_STRING;
    }
  }

  bool arg_is_copy[kTraceMaxNumArgs];
  for (i = 0; i < num_args; ++i) {
    // We only take a copy of arg_vals if they are of type COPY_STRING.
    arg_is_copy[i] = (arg_types_[i] == TRACE_VALUE_TYPE_COPY_STRING);
    if (arg_is_copy[i])
      alloc_size += GetAllocLength(arg_values_[i].as_string);
  }

  if (alloc_size) {
    parameter_copy_storage_ = new char(alloc_size);
    char* ptr = parameter_copy_storage_;
    const char* end = ptr + alloc_size;
    if (copy) {
      CopyTraceEventParameter(&ptr, &name_, end);
      for (i = 0; i < num_args; ++i)
        CopyTraceEventParameter(&ptr, &arg_names_[i], end);
    }
    for (i = 0; i < num_args; ++i) {
      if (arg_is_copy[i])
        CopyTraceEventParameter(&ptr, &arg_values_[i].as_string, end);
    }
    //assert_EQ(end, ptr) << "Overrun by " << ptr - end;
  }
}

event_trace::TraceEvent::~TraceEvent() {
}

void event_trace::TraceEvent::AppendEventsAsJSON(const std::vector<TraceEvent>& events,
                                    size_t start,
                                    size_t count,
                                    std::string* out) {

	for (size_t i = 0; i < count && start + i < events.size(); ++i) {
    if (i > 0)
      *out += ",";
    events[i + start].AppendAsJSON(out);
  }
}

void event_trace::TraceEvent::AppendAsJSON(std::string* out) const {
  int64 time_int64 = timestamp_;
  int process_id = event_trace::TraceLog::GetInstance()->process_id();
  // Category name checked at category creation time.
  assert(!strchr(name_, '"'));
 char buff[1024];

  sprintf(&buff[0],
      "{\"cat\":\"%s\",\"pid\":%i,\"tid\":%i,\"ts\":%lld,"
      "\"ph\":\"%c\",\"name\":\"%s\",\"args\":{",
      event_trace::TraceLog::GetCategoryName(category_enabled_),
      process_id,
      thread_id_,
      time_int64,
      phase_,
      name_);

  out->append(&buff[0]);

  // Output argument names and values, stop at first NULL argument name.
  for (int i = 0; i < kTraceMaxNumArgs && arg_names_[i]; ++i) {
    if (i > 0)
      *out += ",";
    *out += "\"";
    *out += arg_names_[i];
    *out += "\":";
    AppendValueAsJSON(arg_types_[i], arg_values_[i], out);
  }
  *out += "}";

  // If id_ is set, print it out as a hex string so we don't loose any
  // bits (it might be a 64-bit pointer).
  if (flags_ & TRACE_EVENT_FLAG_HAS_ID)
  {
     sprintf(&buff[0], ",\"id\":\"%llu\"", static_cast<uint64>(id_));
	 out->append(&buff[0]);
  }
  *out += "},";
}

////////////////////////////////////////////////////////////////////////////////
//
// TraceLog
//
////////////////////////////////////////////////////////////////////////////////

event_trace::TraceLog event_trace::TraceLog::gStaticTraceInstance;


event_trace::TraceLog::TraceLog()
    : enabled_(false){
  // Trace is enabled or disabled on one thread while other threads are
  // accessing the enabled flag. We don't care whether edge-case events are
  // traced or not, so we allow races on the enabled flag to keep the trace
  // macros fast.
  // TODO(jbates): ANNOTATE_BENIGN_RACE_SIZED crashes windows TSAN bots:
  // ANNOTATE_BENIGN_RACE_SIZED(g_category_enabled, sizeof(g_category_enabled),
  //                            "trace_event category enabled");
  /*for (int i = 0; i < TRACE_EVENT_MAX_CATEGORIES; ++i) {
    ANNOTATE_BENIGN_RACE(&g_category_enabled[i],
                         "trace_event category enabled");
  }*/
#if defined(OS_NACL)  // NaCl shouldn't expose the process id.
  SetProcessID(0);
#else
  SetProcessID(static_cast<int>(getCurrentProcId()));
#endif
}

event_trace::TraceLog::~TraceLog() {
}

const unsigned char* event_trace::TraceLog::GetCategoryEnabled(const char* name) {
  event_trace::TraceLog* tracelog = GetInstance();
  if (!tracelog) {
    assert(!g_category_enabled[g_category_already_shutdown]);
    return &g_category_enabled[g_category_already_shutdown];
  }
  return tracelog->GetCategoryEnabledInternal(name);
}

const char* event_trace::TraceLog::GetCategoryName(const unsigned char* category_enabled) {
  // Calculate the index of the category by finding category_enabled in
  // g_category_enabled array.
  uintptr_t category_begin = reinterpret_cast<uintptr_t>(g_category_enabled);
  uintptr_t category_ptr = reinterpret_cast<uintptr_t>(category_enabled);
  //CLM assert(category_ptr >= category_begin && category_ptr < reinterpret_cast<uintptr_t>(g_category_enabled + TRACE_EVENT_MAX_CATEGORIES)) <<"out of bounds category pointer";
  uintptr_t category_index =
      (category_ptr - category_begin) / sizeof(g_category_enabled[0]);
  return g_categories[category_index];
}

static void EnableMatchingCategory(int category_index,
                                   const std::vector<std::string>& patterns,
                                   unsigned char is_included) {
  std::vector<std::string>::const_iterator ci = patterns.begin();
  bool is_match = false;
  for (; ci != patterns.end(); ++ci) {
    //clm is_match = MatchPattern(event_trace::g_categories[category_index], ci->c_str());
	  is_match = !strcmp(event_trace::g_categories[category_index], ci->c_str());
    if (is_match)
      break;
  }
  event_trace::g_category_enabled[category_index] = is_match ? is_included : (is_included ^ 1);
}

// Enable/disable each category based on the category filters in |patterns|.
// If the category name matches one of the patterns, its enabled status is set
// to |is_included|. Otherwise its enabled status is set to !|is_included|.
static void EnableMatchingCategories(const std::vector<std::string>& patterns,
                                     unsigned char is_included) {
  for (int i = 0; i < event_trace::g_category_index; i++)
    EnableMatchingCategory(i, patterns, is_included);
}

const unsigned char* event_trace::TraceLog::GetCategoryEnabledInternal(const char* name) {
  ScopedLock lock(lock_);
  assert(!strchr(name, '"') &&  "Category names may not contain double quote");

  // Search for pre-existing category matching this name
  for (int i = 0; i < g_category_index; i++) {
    if (strcmp(g_categories[i], name) == 0)
      return &g_category_enabled[i];
  }

  // Create a new category
  //assert(g_category_index < TRACE_EVENT_MAX_CATEGORIES) << "must increase TRACE_EVENT_MAX_CATEGORIES";
  if (g_category_index < TRACE_EVENT_MAX_CATEGORIES) {
    int new_index = g_category_index++;
    g_categories[new_index] = name;
    assert(!g_category_enabled[new_index]);
    if (enabled_) {
      // Note that if both included and excluded_categories are empty, the else
      // clause below excludes nothing, thereby enabling this category.
      if (!included_categories_.empty())
        EnableMatchingCategory(new_index, included_categories_, 1);
      else
        EnableMatchingCategory(new_index, excluded_categories_, 0);
    } else {
      g_category_enabled[new_index] = 0;
    }
    return &g_category_enabled[new_index];
  } else {
    return &g_category_enabled[g_category_categories_exhausted];
  }
}

void event_trace::TraceLog::GetKnownCategories(std::vector<std::string>* categories) {
  ScopedLock lock(lock_);
  for (int i = 0; i < g_category_index; i++)
    categories->push_back(g_categories[i]);
}

void event_trace::TraceLog::SetEnabled(const std::vector<std::string>& included_categories,
                          const std::vector<std::string>& excluded_categories) {
  ScopedLock lock(lock_);
  if (enabled_)
    return;

  logged_events_.reserve(1024);
  enabled_ = true;
  included_categories_ = included_categories;
  excluded_categories_ = excluded_categories;
  // Note that if both included and excluded_categories are empty, the else
  // clause below excludes nothing, thereby enabling all categories.
  if (!included_categories_.empty())
    EnableMatchingCategories(included_categories_, 1);
  else
    EnableMatchingCategories(excluded_categories_, 0);
}

void event_trace::TraceLog::SetEnabled(const std::string& categories) {
  std::vector<std::string> included, excluded;
  // Tokenize list of categories, delimited by ','.
  char* pch = strtok ((char*)categories.c_str(),",");
  while (pch != NULL) {
    bool is_included = true;
    std::string category = pch;
    // Excluded categories start with '-'.
    if (category.at(0) == '-') {
      // Remove '-' from category string.
      category = category.substr(1);
      is_included = false;
    }
    if (is_included)
      included.push_back(category);
    else
      excluded.push_back(category);
	  strtok (NULL,",");
  }
  SetEnabled(included, excluded);
}

void event_trace::TraceLog::GetEnabledTraceCategories(
    std::vector<std::string>* included_out,
    std::vector<std::string>* excluded_out) {
  ScopedLock lock(lock_);
  if (enabled_) {
    *included_out = included_categories_;
    *excluded_out = excluded_categories_;
  }
}

void event_trace::TraceLog::SetDisabled() {
  {
    ScopedLock lock(lock_);
    if (!enabled_)
      return;

    enabled_ = false;
    included_categories_.clear();
    excluded_categories_.clear();
    for (int i = 0; i < g_category_index; i++)
      g_category_enabled[i] = 0;
    AddThreadNameMetadataEvents();
    AddClockSyncMetadataEvents();
  }  // release lock
  Flush();
}

void event_trace::TraceLog::SetEnabled(bool enabled) {
  if (enabled)
    SetEnabled(std::vector<std::string>(), std::vector<std::string>());
  else
    SetDisabled();
}


float event_trace::TraceLog::GetBufferPercentFull() const {
  return (float)((double)logged_events_.size()/(double)kTraceEventBufferSize);
}


void event_trace::TraceLog::Flush() {
  std::vector<TraceEvent> previous_logged_events;
  
  {
    ScopedLock lock(lock_);
    previous_logged_events.swap(logged_events_);
  }  // release lock

  
  for (size_t i = 0;
       i < previous_logged_events.size();
       i += kTraceEventBatchSize) {
     std::string* json_events_str_ptr = new std::string();
    TraceEvent::AppendEventsAsJSON(previous_logged_events,
                                   i,
                                   kTraceEventBatchSize,
                                   json_events_str_ptr);
    event_trace::OutputCallback(json_events_str_ptr->c_str());
  }
}

void event_trace::TraceLog::BeginLogging()
{
	event_trace::TraceLog::GetInstance()->SetEnabled(true);
	event_trace::OutputCallback("[");
}

void event_trace::TraceLog::EndLogging()
{
	event_trace::OutputCallback("]");
}


int event_trace::TraceLog::AddTraceEvent(char phase,
                            const unsigned char* category_enabled,
                            const char* name,
                            unsigned long long id,
                            int num_args,
                            const char** arg_names,
                            const unsigned char* arg_types,
                            const unsigned long long* arg_values,
                            int threshold_begin_id,
                            long long threshold,
                            unsigned char flags) {
  assert(name);
  event_trace::TimeTicks now;
  event_trace::query_counter(&now);

  int ret_begin_id = -1;
  {
    ScopedLock lock(lock_);
    if (!*category_enabled)
      return -1;
    if (logged_events_.size() >= kTraceEventBufferSize)
      return -1;

    int thread_id = static_cast<int>(event_trace::getCurrentThreadId());

    // Record the name of the calling thread, if not done already.
    if (!g_current_thread_name_captured)
	 {
	  //CLM const char* new_name = PlatformThread::GetName();

		 char cur_name[100];
		 sprintf(&cur_name[0],"thread-%i",thread_id);

    // Check if the thread name has been set or changed since the previous
    // call (if any), but don't bother if the new name is empty. Note this will
    // not detect a thread name change within the same char* buffer address: we
    // favor common case performance over corner case correctness.
   // if (new_name != g_current_thread_name.Get().Get() && new_name && *new_name) 
	 //{
    //  g_current_thread_name.Get().Set(new_name);
      std::map<int, std::string>::iterator existing_name = thread_names_.find(thread_id);
      if (existing_name == thread_names_.end()) 
		{
        // This is a new thread id, and a new name.
        thread_names_[thread_id] = cur_name;
      }
		else 
		{
        // This is a thread id that we've seen before, but potentially with a
        // new name.
			/* CLM unsupported for now...
        std::vector<base::StringPiece> existing_names;
        Tokenize(existing_name->second, ",", &existing_names);
        bool found = std::find(existing_names.begin(),
                               existing_names.end(),
                               new_name) != existing_names.end();
        if (!found) {
          existing_name->second.push_back(',');
          existing_name->second.append(new_name);
        }
		 */
      } 
    }

    if (threshold_begin_id > -1) 
	 {
   //   assert(phase == event_trace::TRACE_EVENT_PHASE_END);
      size_t begin_i = static_cast<size_t>(threshold_begin_id);
      // Return now if there has been a flush since the begin event was posted.
      if (begin_i >= logged_events_.size())
        return -1;
      // Determine whether to drop the begin/end pair.
      timer_ticks elapsed = now - logged_events_[begin_i].timestamp();
      if (elapsed < threshold) 
		{
        // Remove begin event and do not add end event.
        // This will be expensive if there have been other events in the
        // mean time (should be rare).
        logged_events_.erase(logged_events_.begin() + begin_i);
        return -1;
      }
    }

    if (flags & TRACE_EVENT_FLAG_MANGLE_ID)
      id ^= process_id_hash_;

    ret_begin_id = static_cast<int>(logged_events_.size());
    logged_events_.push_back(
        TraceEvent(thread_id,
                   now, phase, category_enabled, name, id,
                   num_args, arg_names, arg_types, arg_values,
                   flags));

    if (logged_events_.size() == kTraceEventBufferSize) {
      
    }
  }  // release lock

  
  event_trace::BufferFullCallback();

  return ret_begin_id;
}

void event_trace::TraceLog::AddTraceEventEtw(char phase,
                                const char* name,
                                const void* id,
                                const char* extra) {

  INTERNAL_TRACE_EVENT_ADD(phase, "ETW Trace Event", name,
                           TRACE_EVENT_FLAG_COPY, "id", id, "extra", extra);
}

void event_trace::TraceLog::AddTraceEventEtw(char phase,
                                const char* name,
                                const void* id,
                                const std::string& extra)
{

  INTERNAL_TRACE_EVENT_ADD(phase, "ETW Trace Event", name,
                           TRACE_EVENT_FLAG_COPY, "id", id, "extra", extra);
}

void event_trace::TraceLog::AddClockSyncMetadataEvents() {
#if defined(OS_ANDROID)
  // Since Android does not support sched_setaffinity, we cannot establish clock
  // sync unless the scheduler clock is set to global. If the trace_clock file
  // can't be read, we will assume the kernel doesn't support tracing and do
  // nothing.
  std::string clock_mode;
  if (!file_util::ReadFileToString(
          FilePath("/sys/kernel/debug/tracing/trace_clock"),
          &clock_mode))
    return;

  if (clock_mode != "local [global]\n") {
    DLOG(WARNING) <<
        "The kernel's tracing clock must be set to global in order for " <<
        "trace_event to be synchronized with . Do this by\n" <<
        "  echo global > /sys/kerel/debug/tracing/trace_clock";
    return;
  }

  // Android's kernel trace system has a trace_marker feature: this is a file on
  // debugfs that takes the written data and pushes it onto the trace
  // buffer. So, to establish clock sync, we write our monotonic clock into that
  // trace buffer.
  event_trace::TimeTicks now = event_trace::TimeTicks::HighResNow();

  double now_in_seconds = now.ToInternalValue() / 1000000.0;
  std::string marker =
      StringPrintf("trace_event_clock_sync: parent_ts=%f\n",
                   now_in_seconds);
  if (file_util::WriteFile(
          FilePath("/sys/kernel/debug/tracing/trace_marker"),
          marker.c_str(), marker.size()) == -1) {
    DLOG(WARNING) << "Couldn't write to /sys/kernel/debug/tracing/trace_marker";
    return;
  }
#endif
}

void event_trace::TraceLog::AddThreadNameMetadataEvents() {
  //CLM lock_.AssertAcquired();
  for(std::map<int, std::string>::iterator it = thread_names_.begin();
      it != thread_names_.end();
      it++) {
    if (!it->second.empty()) {
      int num_args = 1;
      const char* arg_name = "name";
      unsigned char arg_type;
      unsigned long long arg_value;
      event_trace::SetTraceValue(it->second, &arg_type, &arg_value);
      logged_events_.push_back(
          TraceEvent(it->first,
                     event_trace::TimeTicks(), TRACE_EVENT_PHASE_METADATA,
                     &g_category_enabled[g_category_metadata],
                     "thread_name", event_trace::kNoEventId,
                     num_args, &arg_name, &arg_type, &arg_value,
                     TRACE_EVENT_FLAG_NONE));
    }
  }
}



void event_trace::TraceLog::SetProcessID(int process_id) {
  process_id_ = process_id;
  // Create a FNV hash from the process ID for XORing.
  // See http://isthe.com/chongo/tech/comp/fnv/ for algorithm details.
  unsigned long long offset_basis = 14695981039346656037ull;
  unsigned long long fnv_prime = 1099511628211ull;
  unsigned long long pid = static_cast<unsigned long long>(process_id_);
  process_id_hash_ = (offset_basis ^ pid) * fnv_prime;
}
