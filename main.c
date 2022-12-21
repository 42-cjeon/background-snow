#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>

typedef struct snow {
  size_t cap;
  size_t width;
  size_t height;
  char *scene;
} Snow;

Snow *snow_new(size_t height, size_t width) {
  Snow *snow = malloc(sizeof(*snow));

  snow->height = height;
  snow->width = width;

  snow->cap = height * width;
  snow->scene = calloc(1, snow->cap * sizeof(*snow->scene));

  return snow;
}

size_t snow_size(Snow * const self) {
  return self->height * self->width;
}

size_t snow_index(Snow * const self, size_t y, size_t x) {
  return y * self->width + x;
}

char *snow_get(Snow *self, size_t y, size_t x) {
  return &self->scene[snow_index(self, y, x)];
}

void snow_del(Snow *self) {
  free(self->scene);
  free(self);
}

void snow_resize(Snow *self, size_t height, size_t width) {
  if (self->cap < snow_size(self)) {
    free(self->scene);

    self->cap = height * width * 2;
    self->scene = calloc(1, self->cap * sizeof(*self->scene));
  } else {
    memset(self->scene, 0, snow_size(self));
  }
  self->height = height;
  self->width = width;
}

void snow_next_step(Snow *self) {
  assert(snow_size(self) /* size must bigger than 0 */);

  for (size_t i = self->height - 1; i > 0; i--) {
    for (size_t j = self->width; j > 0; j--) {
      // overflow protection
      size_t ni = i - 1, nj = j - 1;

      char *from = snow_get(self, ni, nj);
      char *to = snow_get(self, ni + 1, nj);

      *to = *from;
      *from = '\0';
    }
  }

  for (int i = 0; i < self->width; i++) {
    if (rand() % 100 == 0) {
      self->scene[i] = "%&@#"[rand() % 4];
    }
  }
}

void snow_draw(Snow * const self) {
  for (size_t i = 0; i < self->height; i++) {
    for (size_t j = 0; j < self->width; j++) {
      char c = *snow_get(self, i, j);
      move(i, j);
      printw("%c", c ? c : ' ');
    }
  } 
}

WINDOW *main_win;
Snow *snow;

void handle_resize_terminal(int sig) {
  (void)sig;
  snow_resize(snow, LINES, COLS);
}

void handle_int(int sig) {
  (void)sig;

  if (main_win)
    endwin();

  exit(0);
}

#define UPDATE_INTERVAL_MS 1000

// 50ms
#define USLEEP_EPSILON (50 * 1000)

size_t timestamp_ms(void) {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void emsleep(size_t ms) {
  size_t now = timestamp_ms();
  size_t end = now + ms;

  while (now < end) {
    usleep(USLEEP_EPSILON);
    now = timestamp_ms();
  }
}

int main(void) {
  srand(timestamp_ms());

  main_win = initscr();
  snow = snow_new(LINES, COLS);

  signal(SIGWINCH, handle_resize_terminal);
  signal(SIGINT, handle_int);

  while (true) {
    snow_next_step(snow);
    snow_draw(snow);
    refresh();
    emsleep(UPDATE_INTERVAL_MS);
  }

  endwin();
  return 0;
}