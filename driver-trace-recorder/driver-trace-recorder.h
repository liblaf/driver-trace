#ifndef DRIVER_TRACE_RUNTIME_O_H_
#define DRIVER_TRACE_RUNTIME_O_H_

#ifdef __DRIVER_TRACE_MUTEX
#include <linux/mutex.h>  // struct mutex
#endif                    // __DRIVER_TRACE_MUTEX
#include <linux/workqueue.h>
// #include <stdarg.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif  // BUFFER_SIZE

typedef unsigned long long int __DriverTraceInt64T;

// __DriverTraceList

struct __DriverTraceListNode {
  void* data;
  int size;
  struct __DriverTraceListNode* next;
};

struct __DriverTraceList {
  struct __DriverTraceListNode* header;
#ifdef __DRIVER_TRACE_MUTEX
  struct mutex mutex;
#endif  // __DRIVER_TRACE_MUTEX
};

struct __DriverTraceListNode* __DriverTraceListNodeNew(void);
void __DriverTraceListNodeDelete(struct __DriverTraceListNode* self);
void __DriverTraceListNodeInsertAfter(struct __DriverTraceListNode* self,
                                      const void* data, const int count);
void __DriverTraceListNodeDeleteAfter(struct __DriverTraceListNode* self);
struct __DriverTraceListNode* __DriverTraceListFront(
    struct __DriverTraceList* self);
int __DriverTraceListEmpty(const struct __DriverTraceList* self);
void __DriverTraceListPushBack(struct __DriverTraceList* self, const void* data,
                               const int count);
void __DriverTraceListPopFront(struct __DriverTraceList* self);

// __DriverTraceRecoder

void __DriverTraceWriteToBuffer(const void* s, const int count);
static void __DriverTraceLogToFile(struct work_struct* work);

void __DriverTraceOnInit(void);
void __DriverTraceOnCleanup(void);
void __DriverTracePassing(const char* func_name, const char* caller_name,
                          int num_of_params, ...);
void __DriverTraceRecordParameter(const char* type, const void* value);

#endif  // DRIVER_TRACE_RUNTIME_O_H_
