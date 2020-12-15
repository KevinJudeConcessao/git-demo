#include "common.h"
#include <entities.h>
#include <pthread.h>

static void update_status(struct god_t *god, struct subject_t *subject,
                               struct message_t *message) {
  switch (message->id) {
    case AI_HARE:
      god_set_hare_state(god, message->state);
      break;
    case AI_TURTLE:
      god_set_turtle_state(god, message->state);
      break;
  }
}

struct god_t *god_new() {
  struct god_t *self = malloc(sizeof(struct god_t));
  self->hare_state    = READY;
  self->turtle_state  = READY;

  self->observer = observer_new(
      self, (void (*)(void *, struct subject_t *, void *))(update_status));

#ifdef THREADS
  pthread_mutex_init(&self->lock, NULL);
#else
  self->this = subject_new(self);
#endif

  return self;
}

void god_dtor(struct god_t *god) {
  if  (god) {
    observer_dtor(god->observer);
#ifndef THREADS
    subject_dtor(god->this);
#endif
  }
  free(god);
}

void god_set_hare_state(struct god_t *god, enum state_t hare_state) {
  god->hare_state = hare_state;
}

void god_set_turtle_state(struct god_t *god, enum state_t turtle_state) {
  god->turtle_state = turtle_state;
}

#ifndef THREADS
void god_add_to_listener(struct god_t *god, struct observer_t *observer) {
  subject_register_observer(god->this, observer);
}
#endif