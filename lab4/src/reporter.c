#include "common.h"
#include "list.h"
#include <alloca.h>
#include <pthread.h>
#include <string.h>
#include <ncursesw/ncurses.h>
#include <entities.h>
#include <locale.h>
#include <stdarg.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#define TRACK_LENGTH    (1+1+1+1+96+1+1+1+1+1) 
                      /* FLAG + SPACE + START + TRACK + END + SPACE + FLAG + NULL */
#define START_FLAG      L'🏁'
#define START_FLAG_S    L'🏁'
#define END_FLAG        L'🏆'
#define END_FLAG_S      L'🏆'
#define START           L'╠'
#define START_S         L'╠'
#define END             L'╣'
#define END_S           L'╣'
#define TRACK           L'═'
#define TRACK_S         L'═'
#define HARE            L'🐰'
#define HARE_S          L"🐰"
#define TURTLE          L'🐢'
#define TURTLE_S        L"🐢"

#define PERCENT(DISTANCE, LENGTH)   (unsigned int)(((float)(DISTANCE) / (float)(LENGTH)) * 100)

static unsigned int turtle_track_x;
static unsigned int turtle_track_y;

static unsigned int hare_track_x;
static unsigned int hare_track_y;

static wchar_t *status_messages[] = {
  L"Ready to race 💪 !!",
  L"%d km left to complete race. 🏃",
  L"Sleeping 😴💤",
  L"Race Won 🏆 !!",
  L"Race Lost 😓 !!"
};

static WINDOW *window_new(wchar_t *window_name, unsigned int name_attr,
                          int start_y, int start_x, int height, int width) {
  WINDOW *window = newwin(height, width, start_y, start_x);  
  box(window, 0, 0);
  mvwaddwstr(window, 0, 1, window_name);
  wrefresh(window);  
  return window;
}

static void print_track(WINDOW *window, wchar_t animal, unsigned start_y,
                        unsigned start_x, unsigned progress_cent) {
  wchar_t *buffer = alloca(sizeof(wchar_t) * TRACK_LENGTH);
  assert(0 <= progress_cent && progress_cent <= 100);

  buffer[0] = START_FLAG;
  buffer[1] = L' ';
  buffer[2] = L' ';
  buffer[3] = START;

  wmemset(buffer + 4, TRACK, 97);

  buffer[TRACK_LENGTH - 5] = END;
  buffer[TRACK_LENGTH - 4] = L' ';
  buffer[TRACK_LENGTH - 3] = L' ';
  buffer[TRACK_LENGTH - 2] = END_FLAG;
  buffer[TRACK_LENGTH - 1] = L'\0';

  buffer[progress_cent + 2] = animal;
  mvwaddwstr(window, start_y, start_x, buffer);
}

static void update_animal_status(struct reporter *terminal,
                               struct subject_t *animal,
                               struct message_t *message) {
#ifdef THREADS
  reporter_issue_command(terminal, message);
#else
  reporter_render_message(terminal, message);
#endif
}

struct reporter *reporter_new(unsigned int track_distance,
                              struct subject_t *hare, struct subject_t *turtle) {
  struct reporter *self = (struct reporter *)(malloc(sizeof(struct reporter)));
  int center_x, center_y;

  setlocale(LC_ALL, "");
  
  initscr();
  cbreak();
  noecho();

  self->race          = window_new(L"[Race Simulation]", A_BOLD, 0, 0, LINES - 3, COLS);
  self->hare_status   = window_new(L"🐰 ", A_BOLD, LINES - 3, 0, 3, COLS / 2);
  self->turtle_status = window_new(L"🐢 ", A_BOLD, LINES - 3, (COLS / 2) + 1, 3,
                                 COLS - ((COLS / 2) + 1));

  self->track_distance  = track_distance;
  self->hare_progress   = 0;
  self->hare_state      = READY;
  self->turtle_progress = 0;
  self->turtle_state    = READY;

  self->hare_observer = observer_new(
      self,
      (void (*)(void *, struct subject_t *, void *))(update_animal_status));
  self->turtle_observer = observer_new(
      self,
      (void (*)(void *, struct subject_t *, void *))(update_animal_status));

  subject_register_observer(hare, self->hare_observer);
  subject_register_observer(turtle, self->turtle_observer);

#ifdef THREADS
  self->command_queue =
      list_constructor(POINTER, (void (*)(void *))consume_message, NULL);
  pthread_mutex_init(&self->command_lock, NULL);
#endif

  center_x = getmaxx(self->race) / 2;
  center_y = getmaxy(self->race) / 2;

  hare_track_y    = center_y - 5;
  hare_track_x    = center_x - TRACK_LENGTH / 2;
  turtle_track_y  = center_y + 5;
  turtle_track_x  = center_x - TRACK_LENGTH / 2;

  print_track(self->race, HARE, hare_track_y, hare_track_x, 0);
  print_track(self->race, TURTLE, turtle_track_y, turtle_track_x, 0);
  wrefresh(self->race);

  reporter_print_hare_status(self, READY);
  reporter_print_turtle_status(self, READY);

  return self;
}

void reporter_dtor(struct reporter *reporter) {
  if (reporter) {
    endwin();
    observer_dtor(reporter->hare_observer);
    observer_dtor(reporter->turtle_observer);
  #ifdef THREADS
    list_destructor(reporter->command_queue);
  #endif
  }
  free(reporter);
}

void print_status(WINDOW *status_window, wchar_t *message) {
  int end_x   = getmaxx(status_window) - 2;
  int m_len   = wcslen(message);
  int start_x = ((end_x - m_len) / 2) - 2;

  wchar_t *buffer = malloc(sizeof(wchar_t) * (end_x));
  wmemset(buffer, L' ', end_x - 2);
  buffer[end_x - 2] = L'\0';
  wmemcpy(buffer + start_x, message, m_len);
   
  mvwaddwstr(status_window, 1, 1, buffer);
  free(buffer);
  wrefresh(status_window);  
}

void reporter_print_hare_status(struct reporter *reporter, enum state_t state,
                              ...) {
  va_list args;
  unsigned int old_progress;
  unsigned int new_progress;
  unsigned int old_progress_cent;
  unsigned int new_progress_cent;
  unsigned int distance;
  wchar_t *running_status_msg = NULL;
  unsigned len = 0;

  va_start(args, state);
  switch (state) {
  case READY:
    print_status(reporter->hare_status, status_messages[state]);
    reporter->hare_state = state;
    break;

  case SLEEPING:
  case WON:
  case LOST:
    if (state != reporter->hare_state) {
      print_status(reporter->hare_status, status_messages[state]);
      reporter->hare_state = state;
    }
    break;

  case RUNNING:
    old_progress = reporter->hare_progress;
    new_progress = va_arg(args, unsigned int);
    distance = reporter->track_distance;
    assert(!(new_progress > distance));

    old_progress_cent = PERCENT(old_progress, distance);
    new_progress_cent = PERCENT(new_progress, distance);

    if (new_progress_cent == 100) {
      print_track(reporter->race, HARE, hare_track_y, hare_track_x, 100);
      wrefresh(reporter->race);
      reporter_print_hare_status(reporter, WON);
    } 
    else if (new_progress_cent != old_progress_cent) {
      print_track(reporter->race, HARE, hare_track_y, hare_track_x,
                  new_progress_cent);
      wrefresh(reporter->race);

      len = wcslen(status_messages[RUNNING]) + 2;
      running_status_msg = alloca(len * sizeof(wchar_t *));
      swprintf(running_status_msg, len, status_messages[RUNNING],
               distance - new_progress);
      print_status(reporter->hare_status, running_status_msg);
      reporter->hare_state = RUNNING;
    }

    reporter->hare_progress = distance;
    break;
  };

  va_end(args);
}

void reporter_print_turtle_status(struct reporter *reporter, enum state_t state,
                                  ...) {
  va_list args;
  unsigned int old_progress;
  unsigned int new_progress;
  unsigned int old_progress_cent;
  unsigned int new_progress_cent;
  unsigned int distance;
  wchar_t *running_status_msg = NULL;
  unsigned len = 0;

  va_start(args, state);
  switch (state) {
  case READY:
    print_status(reporter->turtle_status, status_messages[state]);
    reporter->turtle_state = state;
    break;

  case SLEEPING:
  case WON:
  case LOST:
    if (state != reporter->turtle_state) {
      print_status(reporter->turtle_status, status_messages[state]);
      reporter->turtle_state = state;
    }
    break;

  case RUNNING:
    old_progress = reporter->turtle_progress;
    new_progress = va_arg(args, unsigned int);
    distance = reporter->track_distance;
    assert(!(new_progress > distance));

    old_progress_cent = PERCENT(old_progress, distance);
    new_progress_cent = PERCENT(new_progress, distance);

    if (new_progress_cent == 100) {
      print_track(reporter->race, TURTLE, turtle_track_y, turtle_track_x, 100);
      wrefresh(reporter->race);
      reporter_print_turtle_status(reporter, WON);
    } 
    else if (new_progress_cent != old_progress_cent) {
      print_track(reporter->race, TURTLE, turtle_track_y, turtle_track_x,
                  new_progress_cent);
      wrefresh(reporter->race);

      len = wcslen(status_messages[RUNNING]) + 2;
      running_status_msg = alloca(len * sizeof(wchar_t *));
      swprintf(running_status_msg, len, status_messages[RUNNING],
               distance - new_progress);
      print_status(reporter->turtle_status, running_status_msg);

      reporter->turtle_state = RUNNING;
    }

    reporter->turtle_progress = distance;
    break;
  };

  va_end(args);
}

#ifdef THREADS
void reporter_issue_command(struct reporter *reporter, struct message_t *message) {
  list_push_back(reporter->command_queue, (list_element) {
    .pointer = new_message(message->id, message->state, message->position)
  });
}

_Bool reporter_has_commands(struct reporter *reporter) {
  return list_size(reporter->command_queue) != 0;
}


void reporter_render_from_queue(struct reporter *reporter) {
  struct message_t *message;

  if (reporter_has_commands(reporter)) {
    message = (struct message_t *)list_iterator_get_data(
                  list_begin(reporter->command_queue))
                  .pointer;
    reporter_render_message(reporter, message);
    list_pop_front(reporter->command_queue);
  }
}
#endif

void reporter_wait_key(struct reporter *reporter) {
  wgetch(reporter->race);
}
/*
const char *get_state_str(enum state_t state) {
  switch (state) {
  case RUNNING:
    return "RUNNING";
  case READY:
    return "READY";
  case WON:
    return "WON";
  case LOST:
    return "LOST";
  case SLEEPING:
    return "SLEEPING";
  }
}
*/
void reporter_render_message(struct reporter *terminal, struct message_t *message) {
  if (message) {
    switch (message->id) {
      case AI_HARE:
//        printf("[HARE]:   %s %d\n", get_state_str(message->state), message->position);
        reporter_print_hare_status(terminal, message->state, message->position);
        break;

      case AI_TURTLE:
//        printf("[TURTLE]:   %s %d\n", get_state_str(message->state), message->position);
        reporter_print_turtle_status(terminal, message->state, message->position);
        break;
    };
  }
}
