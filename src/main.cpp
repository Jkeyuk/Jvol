#include "ui.hpp"
#include <cstdio>
#include <ncurses.h>

int main(int argc, char *argv[]) {
  Jui ui{};

  if (ui.connect() != 0) {
    return 1;
  }
  ui.refresh_state();

  // Start curses mode
  initscr();

  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }

  // Setup Colors
  start_color();
  use_default_colors();
  short fg{0}, bg{0};
  pair_content(COLOR_PAIR(0), &fg, &bg);
  init_pair(1, COLOR_YELLOW, bg);
  init_pair(2, COLOR_GREEN, bg);

  // initiate
  noecho();
  cbreak();
  keypad(stdscr, true);
  curs_set(0);

  while (true) {
    ui.draw();

    int key = getch();
    ui.handle_key(key);
    if (key == 'q') {
      break;
    }
  }

  // End curses mode
  endwin();

  return EXIT_SUCCESS;
}
