#include "driver-trace-recorder.h"

#include "linux/fcntl.h"  // open()
#include "linux/module.h"
#include "linux/sched.h"   // current
#include "linux/slab.h"    // kmalloc(), kfree()
#include "linux/string.h"  // memcpy()

MODULE_LICENSE("Dual BSD/GPL");

static char __driver_trace_buffer[BUFFER_SIZE];
static int __driver_trace_buffer_pos = 0;
static struct __DriverTraceList __driver_trace_buffer_queue;
static struct workqueue_struct* __driver_trace_log_workqueue;
//  int __driver_trace_log_file_descriptor = -1;
static DECLARE_WORK(__driver_trace_log_work, __DriverTraceLogToFile);

// Driver Trace List

struct __DriverTraceListNode* __DriverTraceListNodeNew(void) {
  struct __DriverTraceListNode* new_node =
      kmalloc(sizeof(struct __DriverTraceListNode), GFP_KERNEL);
  new_node->data = kmalloc(BUFFER_SIZE, GFP_KERNEL);
  new_node->size = 0;
  new_node->next = NULL;
  return new_node;
}

void __DriverTraceListNodeDelete(struct __DriverTraceListNode* self) {
  if (self == NULL) return;
  kfree(self->data);
  kfree(self);
}

void __DriverTraceListNodeInsertAfter(struct __DriverTraceListNode* self,
                                      const void* data, const int count) {
  if (self == NULL) return;
  struct __DriverTraceListNode* new_node = __DriverTraceListNodeNew();
  memcpy(new_node->data, data, count);
  new_node->size = count;
  new_node->next = self->next;
  self->next = new_node;
}

void __DriverTraceListNodeEraseAfter(struct __DriverTraceListNode* self) {
  struct __DriverTraceListNode* to_delete = self->next;
  if (to_delete == NULL) return;
  self->next = to_delete->next;
  __DriverTraceListNodeDelete(to_delete);
}

struct __DriverTraceListNode* __DriverTraceListFront(
    struct __DriverTraceList* self) {
  return self->header->next;
}

int __DriverTraceListEmpty(const struct __DriverTraceList* self) {
  return self->header->next == NULL;
}

void __DriverTraceListPushBack(struct __DriverTraceList* self, const void* data,
                               const int count) {
  struct __DriverTraceListNode* iter = self->header;
  while (iter->next) iter = iter->next;
  __DriverTraceListNodeInsertAfter(iter, data, count);
}

void __DriverTraceListPopFront(struct __DriverTraceList* self) {
  __DriverTraceListNodeEraseAfter(self->header);
}

// Driver Trace Recorder

void __DriverTraceWriteToBuffer(const void* s, const int count) {
  int bytes_remain;
  for (bytes_remain = count; bytes_remain > 0;) {
    if (__driver_trace_buffer_pos + bytes_remain >= BUFFER_SIZE) {
      int bytes_to_write = BUFFER_SIZE - __driver_trace_buffer_pos;
      memcpy(__driver_trace_buffer + __driver_trace_buffer_pos, s,
             bytes_to_write);
      __DriverTraceListPushBack(&__driver_trace_buffer_queue,
                                __driver_trace_buffer, BUFFER_SIZE);
      // queue_work(__driver_trace_log_workqueue, &__driver_trace_log_work);
      __driver_trace_buffer_pos = 0;
      bytes_remain -= bytes_to_write;
    } else {
      memcpy(__driver_trace_buffer + __driver_trace_buffer_pos, s,
             bytes_remain);
      bytes_remain = 0;
    }
  }
}

static void __DriverTraceLogFile(struct work_struct* work) {
  struct __DriverTraceListNode* front =
      __DriverTraceListFront(&__driver_trace_buffer_queue);
  void* data = front->data;
  int count = front->size;
}

void __DriverTraceOnInit() {
  __driver_trace_log_workqueue = alloc_ordered_workqueue(
      "__driver_trace_log_workqueue", WQ_FREEZABLE | WQ_MEM_RECLAIM);
}
EXPORT_SYMBOL(__DriverTraceOnInit);

void __DriverTraceOnCleanup() {
  flush_workqueue(__driver_trace_log_workqueue);
  kfree(__driver_trace_log_workqueue);
}
EXPORT_SYMBOL(__DriverTraceOnCleanup);

void __DriverTracePassing(const char* function_name,
                          const char* function_caller_name, int num_of_params,
                          ...) {
  va_list args;
  va_start(args, num_of_params);
  int i;
  printk(KERN_DEBUG "%d;%s;%s;%d;", current->pid, function_name,
         function_caller_name, num_of_params);
  for (i = 0; i < num_of_params; ++i) {
    char* type = va_arg(args, char*);
    va_arg(args, int);
    __DriverTraceRecordParameter(type, NULL);
  }
  va_end(args);
}
EXPORT_SYMBOL(__DriverTracePassing);

void __DriverTraceRecordParameter(const char* type, const void* value) {
  printk(KERN_DEBUG "%d,%s", current->pid, type);
}
