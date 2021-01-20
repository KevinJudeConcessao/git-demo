#ifndef OBSERVER_H
#define OBSERVER_H

#include <list.h>
#include <stddef.h>
#include <stdarg.h>

struct observer_t;
struct subject_t;

#define cleanup_and_return(value, ...)                                         \
  do {                                                                         \
    (void)(__VA_ARGS__, 0);                                                    \
    return value;                                                              \
  } while (0)

#define cleanup_and_return_void(...)                                           \
  do {                                                                         \
    (void)(__VA_ARGS__, 0);                                                    \
    return ;                                                                   \
  } while (0)

#define method_call(ptr, method, ...) ((ptr)->(method)((ptr), __VA_ARGS__))

struct closure_t {
  void *data;
  void (*callback)(void *data, void *return_value);
  void (*data_dtor)(void *);
};

struct closure_t *closure_new(void *data,
                              void (*callback)(void *data, void *return_value),
                              void (*data_dtor)(void *));

void closure_invoke(struct closure_t *closure, void *return_value);
void closure_dtor(struct closure_t *closure);

struct observer_t {
  void *observer_impl;
  void (*notify)(void *observer_impl, struct subject_t *subject, void *data);
};

struct observer_t *observer_new(void *observer_impl,
                                void (*notify_impl)(void *, struct subject_t *,
                                                    void *));

void observer_dtor(struct observer_t *observer);
void observer_notify(struct observer_t *observer, struct subject_t *subject,
                     void *data);

#define observer_impl(observer_ptr) ((observer_ptr)->observer_impl)

struct subject_t {
  void *subject_impl;
  list *observers;
};

struct subject_t *subject_new(void *subject_impl);

void subject_notify(struct subject_t *subject, void *data);

void subject_register_observer(struct subject_t *subject, struct observer_t *observer);
void subject_unregister_observer(struct subject_t *subject, struct observer_t *observer);

void subject_dtor(struct subject_t *subject);

void debug();

#define subject_impl(subject_ptr) ((subject_ptr)->subject_impl)

#endif // OBSERVER_H