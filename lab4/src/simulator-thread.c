#include <pthread.h>
#include <stddef.h>
#include <entities.h>
#include <common.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

struct __args {
  struct animal_t *hare;
  struct animal_t *turtle;
  struct reporter *terminal;
  unsigned int     delta;
  unsigned int     distance;
  struct timespec *step_sleep;
  struct god_t    *god;
};

struct __reporter_thread_args {
  struct reporter *terminal;
  struct timespec *step_sleep;
};

static void hare_thread_runner(struct __args *args) {
  struct animal_t *hare   = args->hare;
  struct animal_t *turtle = args->turtle;
  struct reporter *terminal = args->terminal;
  struct god_t    *god      = args->god;

  enum state_t hare_state;
  unsigned int hare_position;
  enum state_t turtle_state;
  unsigned int turtle_position;

  _Bool finished = false;
  struct timespec hare_sleep_time;

  while (!finished) {
    nanosleep(args->step_sleep, NULL);

    animal_lock(hare);
    hare_state    = animal_get_state(hare);
    hare_position = animal_get_position(hare);
    animal_unlock(hare);

    switch (hare_state) {
      case READY:
      case RUNNING:
        hare_position += 1;
        animal_lock(turtle);
        turtle_state    = animal_get_state(turtle);
        turtle_position = animal_get_position(turtle);
        animal_unlock(turtle);

        switch (turtle_state) {
          case READY:
          case RUNNING:
          case SLEEPING:
            hare_state = (hare_position == args->distance)
                             ? WON
                             : (((int)(hare_position) - (int)(turtle_position)) > (int)(args->delta)
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
    god_lock(god); 
    animal_lock(hare);
    reporter_lock_command(terminal);       

    animal_set(hare, hare_state, hare_position);     
    
    reporter_unlock_command(terminal);
    animal_unlock(hare);
    god_unlock(god);
  }

  pthread_exit(NULL);
}

static void *turtle_thread_runner(struct __args *args) {
  struct animal_t *hare     = args->hare;
  struct animal_t *turtle   = args->turtle;
  struct reporter *terminal = args->terminal;
  struct god_t    *god      = args->god;

  enum state_t turtle_state;
  unsigned int turtle_position;
  enum state_t hare_state;

  _Bool finished = false;

  while (!finished) {
    nanosleep(args->step_sleep, NULL);

    animal_lock(turtle);
    turtle_state    = animal_get_state(turtle);
    turtle_position = animal_get_position(turtle);
    animal_unlock(turtle);

    switch (turtle_state) {
      case READY:
      case RUNNING:
        turtle_position += 1;
        animal_lock(hare);
        hare_state = animal_get_state(hare);
        animal_unlock(hare);

        switch (hare_state) {
          case READY:
          case RUNNING:
          case SLEEPING:
            turtle_state = (turtle_position == args->distance) ? WON : RUNNING;
            break;

          case WON:
            turtle_state = LOST;
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

      default:
        assert(0 && "Invalid State!!");
    }

    god_lock(god);  
    animal_lock(turtle);
    reporter_lock_command(terminal);      

    animal_set(turtle, turtle_state, turtle_position);      
    
    reporter_unlock_command(terminal);
    animal_unlock(turtle);
    god_unlock(god);
  }

  pthread_exit(NULL);
}

static void *reporter_thread_runner(struct __reporter_thread_args *args) {
  _Bool finished = false;
  struct message_t *message = NULL;
  struct message_t *other_message = NULL;

  enum animal_id other_id;
  enum state_t   other_state;

  while (!finished) {
    reporter_lock_command(args->terminal);
    if (reporter_has_commands(args->terminal)) {
      message = (struct message_t *)list_iterator_get_data(
                    list_begin(args->terminal->command_queue))
                    .pointer;
      reporter_render_message(args->terminal, message);

      if (message->state == WON || message->state == LOST) {
        if (message->state == WON)
          other_state = LOST;
        else if (message->state == LOST)
          other_state = WON;

        if (message->id == AI_TURTLE)
          other_id = AI_HARE;
        else if (message->id == AI_HARE)
          other_id = AI_TURTLE;

        other_message = new_message(other_id, other_state, 0);
        reporter_render_message(args->terminal, other_message);
        consume_message(other_message);
        finished = true;
      } 
    }
    reporter_unlock_command(args->terminal);
    nanosleep(args->step_sleep, NULL);
  }
  pthread_exit(NULL);
}

void *god_thread_runner(struct __args *args) {
  _Bool finished = false;
  enum state_t hare_state;
  enum state_t turtle_state;
  int random;

  struct animal_t *hare     = args->hare;
  struct animal_t *turtle   = args->turtle;
  struct reporter *terminal = args->terminal;
  struct god_t    *god      = args->god;

  while (!finished) {
    god_lock(god);
    hare_state = god_get_hare_state(god);
    if (hare_state == WON || hare_state == LOST)
      finished = true;
    else {
      random = rand() % 1000;
      if (random >= 211 && random <= 218) {
        if (hare_state == RUNNING || hare_state == SLEEPING) {                  
          animal_lock(hare);
          reporter_lock_command(terminal);  
          animal_set(hare, hare_state, 1 + (rand() % (args->distance - 2)));
          reporter_unlock_command(terminal);
          animal_unlock(hare);          
        }
      }
    }
    god_unlock(god);

    if (finished)
      goto quit;
    else
      nanosleep(args->step_sleep, NULL);

    god_lock(god);    
    turtle_state = god_get_turtle_state(god);
    if (turtle_state == WON || turtle_state == LOST)
      finished = true;
    else {
      random = rand() % 1000;
      if (random >= 911 && random <= 918) {
        if (turtle_state == RUNNING || turtle_state == SLEEPING) {
          animal_lock(turtle);
          reporter_lock_command(terminal);
          animal_set(turtle, turtle_state, 1 + (rand() % (args->distance - 2)));
          reporter_unlock_command(terminal);
          animal_unlock(turtle);
        }
      }
    }
    god_unlock(god);

    if (finished)
      goto quit;
    else
      nanosleep(args->step_sleep, NULL);
  }
quit:
  pthread_exit(NULL);
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

void simulator_main(unsigned distance, struct timespec *hare_step_sleep,
                    struct timespec *turtle_step_sleep, unsigned delta) {
  struct animal_t *hare = animal_new(AI_HARE, "hare", NULL);
  struct animal_t *turtle = animal_new(AI_TURTLE, "turtle", NULL);
  struct reporter *reporter = reporter_new(distance, hare->this, turtle->this);
  struct god_t *god = god_new();

  struct __args hare_thread_args = { 
    .hare       = hare, 
    .turtle     = turtle, 
    .terminal   = reporter,
    .delta      = delta,
    .distance   = distance,
    .step_sleep = hare_step_sleep
  };

  struct __args turtle_thread_args = { 
    .hare       = hare, 
    .turtle     = turtle, 
    .terminal   = reporter,
    .delta      = delta,
    .distance   = distance,
    .step_sleep = turtle_step_sleep
  };

  struct __reporter_thread_args r_args = {
    .terminal   = reporter,
    .step_sleep = min_time(hare_step_sleep, turtle_step_sleep),
  };

  struct __args g_args = {
    .distance   = distance,
    .god        = god,
    .hare       = hare,
    .turtle     = turtle,
    .terminal   = reporter,
    .step_sleep = max_time(hare_step_sleep, turtle_step_sleep)
  };

  pthread_t hare_thread;
  pthread_t turtle_thread;
  pthread_t reporter_thread;
  pthread_t god_thread;

  srand(getpid());

  animal_add_to_listener(turtle, hare->other);
  animal_add_to_listener(hare, turtle->other);
  animal_add_to_listener(turtle, god->observer);
  animal_add_to_listener(hare, god->observer);

  pthread_create(&hare_thread, NULL, (void *(*)(void *))(hare_thread_runner),
                 &hare_thread_args);
  pthread_create(&turtle_thread, NULL,
                 (void *(*)(void *))(turtle_thread_runner),
                 &turtle_thread_args);
  pthread_create(&reporter_thread, NULL,
                 (void *(*)(void *))(reporter_thread_runner), &r_args);
  pthread_create(&god_thread, NULL, (void *(*)(void *))(god_thread_runner),
                 &g_args);

  pthread_join(hare_thread, NULL);
  pthread_join(turtle_thread, NULL);
  pthread_join(reporter_thread, NULL);
  pthread_join(god_thread, NULL);

  reporter_wait_key(reporter);
  god_dtor(god);
  reporter_dtor(reporter);
  animal_dtor(turtle);
  animal_dtor(hare);
}