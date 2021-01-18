#include <list.h>
#include <common.h>
#include <stddef.h>
#include <stdarg.h>
#include <entities.h>

struct animal_t *animal_new(enum animal_id id, char *name,
      void (*action)(struct animal_t *, struct subject_t *, struct message_t *)) {
        
  struct animal_t *self = (struct animal_t *)(malloc(sizeof(struct animal_t)));
  self->id        = id;
  self->state     = READY;
  self->position  = 0;
  self->name      = name;

  self->this  = subject_new(self);
  self->other = observer_new(
      self, (void (*)(void *, struct subject_t *, void *))(action));

#ifdef THREADS
  pthread_mutex_init(&self->lock, NULL);
#endif

  return self;
}

void animal_dtor(struct animal_t *animal) {
  if (animal) {
    subject_dtor(animal->this);
    observer_dtor(animal->other);
  }
  free(animal);
}

void animal_set(struct animal_t *animal, enum state_t state, ...) {
  va_list args;
  struct message_t *message = NULL;

  va_start(args, state);

  animal->state = state;
  switch (state) {
    case READY:
      animal->position = 0;
      break; 

    case SLEEPING:
    case RUNNING:
      animal->position = va_arg(args, unsigned int);
      break; 

    case WON:
    case LOST:
      break; 
  };

  message = new_message(animal->id, animal->state, animal->position);

  if (animal->other) {
    subject_notify(animal->this, message);
    consume_message(message);
  }

  va_end(args);
}

void animal_add_to_listener(struct animal_t *animal, struct observer_t *observer) {
  subject_register_observer(animal->this, observer);
}