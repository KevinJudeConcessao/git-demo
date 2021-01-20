#include "entities.h"
#include <list.h>
#include <common.h>
#include <execinfo.h>
#include <unistd.h>

struct closure_t *closure_new(void *data,
                              void (*callback)(void *, void *),
                              void (*data_dtor)(void *)) {
  struct closure_t *self = calloc(1, sizeof(struct closure_t));
  self->data      = data;
  self->callback  = callback;
  self->data_dtor = data_dtor;
  return self;
}

void closure_invoke(struct closure_t *closure, void *return_value) {
  closure->callback(closure->data, return_value);
}

void closure_dtor(struct closure_t *closure) {
  if (closure->data_dtor)
    closure->data_dtor(closure->data);
  free(closure);
}

struct observer_t *observer_new(void *observer_impl,
                                void (*notify_impl)(void *, struct subject_t *,
                                                    void *)) {
  struct observer_t *self = calloc(1, sizeof(struct observer_t));

  self->observer_impl = observer_impl;
  self->notify        = notify_impl;
  return self;
}

void observer_dtor(struct observer_t *observer) {
  free(observer);
}

void observer_notify(struct observer_t *observer, struct subject_t *subject, void *data) {
  if (observer->notify)
    observer->notify(observer->observer_impl, subject, data);
}

struct subject_t *subject_new(void *subject_impl) {
  struct subject_t *self = calloc(1, sizeof(struct subject_t));

  self->observers     = list_constructor(POINTER, NULL, NULL);
  self->subject_impl  = subject_impl;
  return self;
}

void subject_notify(struct subject_t *subject, void *data) {
  list_iterator iter, last;
  struct observer_t *observer;

  for (iter = list_begin(subject->observers),
      last = list_end(subject->observers);
       !list_iterator_is_same(iter, last); list_iterator_next(&iter)) {
       
    observer = list_iterator_get_data(iter).pointer;
    
    if (observer && observer->observer_impl && observer->notify) {
      observer->notify(observer->observer_impl, subject, data);
    }
  }
}

void subject_register_observer(struct subject_t *subject, struct observer_t *observer) {
  list_iterator iter = list_begin(subject->observers);
  list_iterator end  = list_end(subject->observers);

  while (!list_iterator_is_same(iter, end) &&
         list_iterator_get_data(iter).pointer != observer)         
    list_iterator_next(&iter);

  if (list_iterator_is_same(iter, end))
    list_insert(end, (list_element) {
      .pointer = observer
    });
}

void subject_dtor(struct subject_t *subject) {
  if (subject)
    list_destructor(subject->observers);
  free(subject);
}

void debug() {
  void *fptrs[1024];
  char **strings = NULL;
  int size;

  size = backtrace(fptrs, 1024);
  strings = backtrace_symbols(fptrs, size);

  if (strings) {
    printf("[%d] Stacktrace:\n", getpid());
    for (int i = 1; i < size; ++i) {
      printf ("%s\n", strings[i]);
    }
  }
}