#ifndef ENTITIES_H
#define ENTITIES_H

#include <list.h>
#include <stdarg.h>
#include <ncursesw/ncurses.h>
#include <pthread.h>
#include <common.h>

enum state_t {
  READY,
  RUNNING,
  SLEEPING,
  WON,
  LOST
};

enum animal_id {
  AI_HARE = 1,
  AI_TURTLE
};

struct message_t {
  enum animal_id  id;
  enum state_t    state;
  unsigned int    position;
};

struct message_t *new_message(enum animal_id id, enum state_t state,
                              unsigned int position);
void consume_message(struct message_t *);

#ifndef THREADS

struct msg {
  long mtype;
  struct message_t message;
};

void msg_init(struct msg *m, long type, enum animal_id id, enum state_t state,
              unsigned int position);

#define id(msg)       msg.message.id
#define state(msg)    msg.message.state
#define position(msg) msg.message.position

struct comm_stub_t {
  int   msgqid;
  long  type;
  struct observer_t *observer;
  struct subject_t  *this;
};

struct comm_stub_t *comm_stub_new(int msgqid, long type);
void comm_stub_dtor(struct comm_stub_t *comm_stub);

void comm_stub_send(struct comm_stub_t *comm_stub, enum animal_id id,
                    enum state_t state, unsigned int position);
int comm_stub_receive(struct comm_stub_t *comm_stub, struct message_t *messag, _Bool wait);
int comm_stub_receive_notify(struct comm_stub_t *comm_stub,
                             struct message_t *message, _Bool wait);

int comm_stub_receive_notify_last(struct comm_stub_t *comm_stub,
                                  struct message_t *message);

int comm_stub_receive_last(struct comm_stub_t *comm_stub, struct message_t *message);
void comm_stub_add_to_listener(struct comm_stub_t *comm_stub,
                               struct observer_t *observer);

#endif

struct animal_t {
  enum animal_id id;
  enum state_t state;
  unsigned int position;
  char *name;

  struct subject_t  *this;
  struct observer_t *other;

#ifdef THREADS
  pthread_mutex_t lock;
#endif
};

struct animal_t *animal_new(enum animal_id id, char *name,
                            void (*action)(struct animal_t *,
                                           struct subject_t *,
                                           struct message_t *));

void animal_dtor(struct animal_t *animal);

void animal_set(struct animal_t *animal, enum state_t state, ...);
void animal_add_to_listener(struct animal_t *animal, struct observer_t *observer);

#define animal_get_position(animal_ptr) (animal_ptr)->position
#define animal_get_state(animal_ptr)    (animal_ptr)->state

#ifdef THREADS
  #define animal_lock(animal_ptr)         pthread_mutex_lock(&(animal_ptr)->lock)
  #define animal_unlock(animal_ptr)       pthread_mutex_unlock(&(animal_ptr)->lock)
#endif

struct reporter {
  WINDOW *race;
  WINDOW *turtle_status;
  WINDOW *hare_status;

  /* Cached for efficiency */
  unsigned int turtle_progress;
  unsigned int hare_progress;
  unsigned int track_distance;

  /* Cached for efficiency */
  enum state_t turtle_state;
  enum state_t hare_state;

  struct observer_t *hare_observer;
  struct observer_t *turtle_observer;

#ifdef THREADS
  list *command_queue;
  pthread_mutex_t command_lock;
#endif
};

struct reporter *reporter_new(unsigned int track_distance,
                              struct subject_t *hare, struct subject_t *turtle);
void reporter_dtor(struct reporter *reporter);

void reporter_print_hare_status(struct reporter *reporter, enum state_t state,
                                ...);
void reporter_print_turtle_status(struct reporter *reporter, enum state_t state,
                                  ...);

void reporter_render_message(struct reporter *terminal, struct message_t *message);

#ifdef THREADS
  void reporter_issue_command(struct reporter *reporter, struct message_t *message);
  _Bool reporter_has_commands(struct reporter *reporter);
  void reporter_render_from_queue(struct reporter *reporter, struct message_t *message);
#endif

void reporter_wait_key(struct reporter *reporter);

#ifdef THREADS
  #define reporter_lock_command(reporter_ptr)     pthread_mutex_lock(&(reporter_ptr)->command_lock)
  #define reporter_unlock_command(reporter_ptr)   pthread_mutex_unlock(&(reporter_ptr)->command_lock)
#endif

struct god_t {
  enum state_t hare_state;
  enum state_t turtle_state;

  struct observer_t *observer;

#ifdef THREADS
  pthread_mutex_t lock;
#else
  struct subject_t *this;
#endif
};

struct god_t *god_new();
void god_dtor(struct god_t *god);

#ifndef THREADS
  void god_add_to_listener(struct god_t *god, struct observer_t *observer);
#endif

void god_set_hare_state(struct god_t *god, enum state_t hare_state);
void god_set_turtle_state(struct god_t *god, enum state_t turtle_state);

#define god_get_hare_state(god_ptr)    (god_ptr)->hare_state
#define god_get_turtle_state(god_ptr)  (god_ptr)->turtle_state

#ifdef THREADS
  #define god_lock(god_ptr)    pthread_mutex_lock(&(god_ptr)->lock)
  #define god_unlock(god_ptr)  pthread_mutex_unlock(&(god_ptr)->lock)
#endif

#endif // ENTITIES_H