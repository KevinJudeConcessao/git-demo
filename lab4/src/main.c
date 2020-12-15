#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

extern void simulator_main(unsigned distance, struct timespec *hare_step_sleep,
                           struct timespec *turtle_step_sleep, unsigned delta);

static void convert(unsigned speed, struct timespec *step_sleep) {
  double time_per_km  = 1.0 / (double)(speed);
  step_sleep->tv_sec  = (unsigned long)(time_per_km);
  step_sleep->tv_nsec =
      (unsigned long)((time_per_km - step_sleep->tv_sec) * 1e9);
}

void usage() {
  setlocale(LC_ALL, "");
  wprintf(L"Usage:" "\n");
#ifdef THREADS
  wprintf(L"./sim-threads [distance] [hare-speed] [turtle-speed] [delta]" "\n");
#else
  wprintf(L"./sim-process [distance] [hare-speed] [turtle-speed] [delta]" "\n");
#endif
  wprintf(L"  [distance]        distance in meters" "\n");
  wprintf(L"  [hare-speed]      speed of the hare in ms⁻¹" "\n");
  wprintf(L"  [turtle-speed]    speed of the turtle in ms⁻¹" "\n");
  wprintf(L"  [delta]           max acceptable distance between hare" "\n");
  wprintf(L"                    and turtle" "\n");
}

int main(int argc, char *argv[]) {
  unsigned distance;
  unsigned hare_speed;
  unsigned turtle_speed;
  unsigned delta;

  struct timespec hare_step_sleep;
  struct timespec turtle_step_sleep;

  if (argc != 5) {
    usage();
    exit(1);
  }

  distance      = atoi(argv[1]);
  hare_speed    = atoi(argv[2]);
  turtle_speed  = atoi(argv[3]);
  delta         = atoi(argv[4]);

  convert(hare_speed, &hare_step_sleep);
  convert(turtle_speed, &turtle_step_sleep);
  simulator_main(distance, &hare_step_sleep, &turtle_step_sleep, delta);

  return 0;
}