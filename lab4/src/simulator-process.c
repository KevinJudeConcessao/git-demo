#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <entities.h>
#include <common.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/signal.h>

#define DEFAULT_Q 100

enum {
  TO_HARE,
  TO_TURTLE,
  TO_GOD,
  TO_REPORTER,
  Q_MAX
};

char *msg_queues[Q_MAX] = {
    "TO_HARE",
    "TO_TURTLE",
    "TO_GOD",
    "TO_REPORTER",
};

int msq_ids[Q_MAX] = { 0 };

static void update_turtle(struct animal_t *turtle, struct subject_t *subject,
                          struct message_t *message) {
  enum state_t state = animal_get_state(turtle);
  if (message->id == AI_TURTLE && (state == RUNNING || state == SLEEPING)) {
    animal_set(turtle, state, message->position);
  }
}

static void update_hare(struct animal_t *hare, struct subject_t *subject,
                        struct message_t *message) {
  enum state_t state = animal_get_state(hare);
  if (message->id == AI_HARE && (state == RUNNING || state == SLEEPING)) {
    animal_set(hare, state, message->position);
  }
}

static void hare_process(unsigned distance, unsigned delta,
                         struct timespec *step_sleep, int to_hare_msgqid,
                         int to_turtle_msgqid, int to_god_msgqid,
                         int to_terminal_msgqid) {
  struct animal_t *hare = animal_new(AI_HARE, "hare", update_hare);
  struct animal_t *turtle = animal_new(AI_TURTLE, "turtle", update_turtle);
  struct comm_stub_t *god_to_hare       = comm_stub_new(to_hare_msgqid, AI_HARE);
  struct comm_stub_t *turtle_to_hare    = comm_stub_new(to_hare_msgqid, AI_TURTLE);
  struct comm_stub_t *hare_to_god       = comm_stub_new(to_god_msgqid, AI_HARE);
  struct comm_stub_t *hare_to_terminal  = comm_stub_new(to_terminal_msgqid, AI_HARE);
  struct comm_stub_t *hare_to_turtle    = comm_stub_new(to_turtle_msgqid, AI_HARE);

  struct message_t message;
  _Bool finished = false;
  int ret = 0;

  enum state_t hare_state;
  unsigned int hare_position;
  enum state_t turtle_state;
  unsigned int turtle_position;
  struct timespec hare_sleep_time;

  animal_add_to_listener(hare, hare_to_god->observer);
  animal_add_to_listener(hare, hare_to_terminal->observer);
  animal_add_to_listener(hare, hare_to_turtle->observer);

  comm_stub_add_to_listener(god_to_hare, hare->other);
  comm_stub_add_to_listener(turtle_to_hare, turtle->other);
  
  while (!finished) {
    nanosleep(step_sleep, NULL);
    ret = comm_stub_receive_notify_last(god_to_hare, &message);
    hare_state    = animal_get_state(hare);
    hare_position = animal_get_position(hare);

    switch (hare_state) {
      case READY:
      case RUNNING:
        hare_position += 1;
        ret = comm_stub_receive_notify_last(turtle_to_hare, &message);
        turtle_state    = animal_get_state(turtle);
        turtle_position = animal_get_position(turtle);

        switch (turtle_state) {
          case READY:
          case RUNNING:
          case SLEEPING:
            hare_state =
                (hare_position == distance)
                    ? WON
                    : (((int)(hare_position) - (int)(turtle_position)) > (int)(delta)
                           ? SLEEPING /* (rand() % 2 ? RUNNING : SLEEPING) */
                           : RUNNING);
            break;

          case WON:
            hare_state = LOST;
            finished = true;
            break;

          case LOST:
            assert(0 && "turtle cannot declare defeat on its own !!");
        }
        break;

      case WON:
      case LOST:
        finished = true;
        break;

      case SLEEPING:
        hare_sleep_time = (struct timespec) {
          .tv_sec   = rand() % 3,
          .tv_nsec  = rand() % (unsigned int)(1e9),
        };
        nanosleep(&hare_sleep_time, NULL);
        hare_state = RUNNING;
        break;
    }
    animal_set(hare, hare_state, hare_position);         
  }

  comm_stub_dtor(god_to_hare);
  comm_stub_dtor(turtle_to_hare);
  comm_stub_dtor(hare_to_god);
  comm_stub_dtor(hare_to_terminal);
  comm_stub_dtor(hare_to_turtle);
  animal_dtor(hare);
  animal_dtor(turtle);
}

static void turtle_process(unsigned distance, struct timespec *step_sleep,
                           int to_turtle_msgqid, int to_hare_msgqid,
                           int to_god_msgqid, int to_terminal_msgqid) {
  struct animal_t *turtle = animal_new(AI_TURTLE, "turtle", update_turtle);
  struct animal_t *hare   = animal_new(AI_HARE, "hare", update_hare);
  struct comm_stub_t *god_to_turtle       = comm_stub_new(to_turtle_msgqid, AI_TURTLE);
  struct comm_stub_t *hare_to_turtle      = comm_stub_new(to_turtle_msgqid, AI_HARE);
  struct comm_stub_t *turtle_to_god       = comm_stub_new(to_god_msgqid, AI_TURTLE);
  struct comm_stub_t *turtle_to_terminal  = comm_stub_new(to_terminal_msgqid, AI_TURTLE);
  struct comm_stub_t *turtle_to_hare      = comm_stub_new(to_hare_msgqid, AI_TURTLE);

  struct message_t message;
  int ret;

  enum state_t turtle_state;
  unsigned int turtle_position;
  enum state_t hare_state;

  _Bool finished = false;

  animal_add_to_listener(turtle, turtle_to_god->observer);
  animal_add_to_listener(turtle, turtle_to_hare->observer);
  animal_add_to_listener(turtle, turtle_to_terminal->observer);

  comm_stub_add_to_listener(god_to_turtle, turtle->other);
  comm_stub_add_to_listener(hare_to_turtle, hare->other);

  while (!finished) {
    nanosleep(step_sleep, NULL);
    ret = comm_stub_receive_notify_last(god_to_turtle, &message);
    turtle_state    = animal_get_state(turtle);
    turtle_position = animal_get_position(turtle);

    switch (turtle_state) {
      case READY:
      case RUNNING:
        turtle_position += 1;
        ret = comm_stub_receive_notify_last(hare_to_turtle, &message);
        hare_state = animal_get_state(hare);

        switch (hare_state) {
          case READY:
          case RUNNING:
          case SLEEPING:
            turtle_state = (turtle_position == distance) ? WON : RUNNING;
            break;

          case WON:
            turtle_state = LOST;
            finished = true;
            break;

          case LOST:
            assert(0 && "hare cannot declare defeat on its own !!");
        }
        break;

      case WON:
      case LOST:
        finished = true;
        break;

      default:
        assert(0 && "Invalid State!!");
    }
    animal_set(turtle, turtle_state, turtle_position);   
  } 

  comm_stub_dtor(turtle_to_hare);
  comm_stub_dtor(turtle_to_terminal);
  comm_stub_dtor(turtle_to_god);
  comm_stub_dtor(hare_to_turtle);
  comm_stub_dtor(god_to_turtle);
  animal_dtor(turtle);
  animal_dtor(hare);
}

static void animal_notify(struct animal_t *animal, struct subject_t *subject,
                          struct message_t *message) {
  animal_set(animal, message->state, animal->position);
}

static void terminal_process(unsigned distance, int to_terminal_msgqid,
                             struct timespec *step_sleep) {
  struct comm_stub_t *hare_messages = comm_stub_new(to_terminal_msgqid, AI_HARE);
  struct comm_stub_t *turtle_messages =
      comm_stub_new(to_terminal_msgqid, AI_TURTLE);

  struct animal_t *hare   = animal_new(AI_HARE, "hare", animal_notify);
  struct animal_t *turtle = animal_new(AI_TURTLE, "turtle", animal_notify);

  struct reporter *reporter = reporter_new(distance, hare->this, turtle->this);
 
  struct message_t message;
  _Bool finished = false;
  int ret = 0;

  comm_stub_add_to_listener(hare_messages, hare->other);
  comm_stub_add_to_listener(turtle_messages, turtle->other);

  while (!finished) {
    ret = comm_stub_receive_notify(hare_messages, &message, false);
    if (ret != -1) {
      if (message.state == WON) {
        finished = true;
        animal_set(turtle, LOST);
      }
    }
    
    if (!finished) {
      ret = comm_stub_receive_notify(turtle_messages, &message, false);
      if (ret != -1) {
        if (message.state == WON) {
          finished = true;
          animal_set(hare, LOST);
        }
      }
    }
    nanosleep(step_sleep, NULL);
  }
  
  reporter_wait_key(reporter);
  reporter_dtor(reporter);
  animal_dtor(turtle);
  animal_dtor(hare);
  comm_stub_dtor(turtle_messages);
  comm_stub_dtor(hare_messages);
}

static void god_process(unsigned distance, struct timespec *god_step_sleep,
                        int to_god_msgqid, int to_turtle_msgqid,
                        int to_hare_msgqid) {
  struct comm_stub_t *hare_to_god     = comm_stub_new(to_god_msgqid, AI_HARE);
  struct comm_stub_t *turtle_to_god   = comm_stub_new(to_god_msgqid, AI_TURTLE);
  struct comm_stub_t *to_turtle       = comm_stub_new(to_turtle_msgqid, AI_TURTLE);
  struct comm_stub_t *to_hare         = comm_stub_new(to_hare_msgqid, AI_HARE);
  
  struct message_t message;
  int random;
  _Bool finished = false;
  int ret = 0;

  int r1 = rand() % 1000;
  int r2 = rand() % 1000;

  srand(getpid());

  while (!finished) {
    ret = comm_stub_receive(hare_to_god, &message, false);
    if (ret != -1) {
      if (message.state == WON || message.state == LOST)
        finished = true;
      else {
        random = rand() % 1000;
        if (random >= r1 && random <= (r1 + 8)) {
          if (message.state == RUNNING || message.state == SLEEPING)
            comm_stub_send(to_hare, AI_HARE, message.state,
                           1 + (rand() % (distance - 2)));
        }
      }
    }

    ret = comm_stub_receive(turtle_to_god, &message, false);
    if (ret != -1) {
      if (message.state == WON || message.state == LOST)
        finished = true;
      else {
        random = rand() % 1000;
        if (random >= r2 && random <= (r2 + 8)) {
          if (message.state == RUNNING || message.state == SLEEPING)
            comm_stub_send(to_turtle, AI_TURTLE, message.state,
                           1 + (rand() % (distance - 2)));
        }
      }
    }

    if (!finished)
      nanosleep(god_step_sleep, NULL);
  }

  comm_stub_dtor(to_hare);
  comm_stub_dtor(to_turtle);
  comm_stub_dtor(turtle_to_god);
  comm_stub_dtor(hare_to_god);
}

static struct timespec *min_time(struct timespec *T1, struct timespec *T2) {
  if (T1->tv_sec < T2->tv_sec)
    return T1;
  else if (T1->tv_sec == T2->tv_sec)
    return (T1->tv_nsec < T2->tv_nsec ? T1 : T2);
  else
    return T2;
}

static struct timespec *max_time(struct timespec *T1, struct timespec *T2) {
  return min_time(T1, T2) == T1 ? T2 : T1;
}

static void do_cleanup(int signal) {
  int i;
  for (i = 0; i < Q_MAX; ++i) {
    msgctl(msq_ids[i], IPC_RMID, NULL);
    unlink(msg_queues[i]);
  }
  exit(-1);
}

void simulator_main(unsigned distance, struct timespec *hare_step_sleep,
                    struct timespec *turtle_step_sleep, unsigned delta)  {
  int hare_pid;
  int turtle_pid;
  int terminal_pid;
  int fd;
  key_t key;
  int i;
  
  for (i = 0; i < Q_MAX; ++i) {
    fd = creat(msg_queues[i], S_IRUSR | S_IWUSR);
    msq_ids[i] = msgget(ftok(msg_queues[i], getpid()), IPC_CREAT | 0666);
    close(fd);
  }

  signal(SIGINT,  do_cleanup);
  signal(SIGTERM, do_cleanup);
  signal(SIGQUIT, do_cleanup);

  if ((hare_pid = fork()) == 0) {
    raise(SIGSTOP);
    hare_process(distance, delta, hare_step_sleep, msq_ids[TO_HARE],
                 msq_ids[TO_TURTLE], msq_ids[TO_GOD], msq_ids[TO_REPORTER]);
    return;
  } 

  if ((turtle_pid = fork()) == 0) {
    raise(SIGSTOP);
    turtle_process(distance, turtle_step_sleep, msq_ids[TO_TURTLE],
                   msq_ids[TO_HARE], msq_ids[TO_GOD], msq_ids[TO_REPORTER]);
    return;
  }

  if ((terminal_pid = fork()) == 0) {
    raise(SIGSTOP);
    terminal_process(distance, msq_ids[TO_REPORTER],
                     min_time(hare_step_sleep, turtle_step_sleep));
    return;
  }

  kill(terminal_pid, SIGCONT);
  kill(turtle_pid, SIGCONT);
  kill(hare_pid, SIGCONT);

  god_process(distance, max_time(hare_step_sleep, turtle_step_sleep),
              msq_ids[TO_GOD], msq_ids[TO_TURTLE], msq_ids[TO_HARE]);

  waitpid(terminal_pid, NULL, 0);
  waitpid(hare_pid, NULL, 0);
  waitpid(turtle_pid, NULL, 0);

cleanup:
  do_cleanup(0);
}