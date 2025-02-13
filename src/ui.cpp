#include "ui.hpp"
#include <algorithm>
#include <cstring>
#include <ncurses.h>

const auto DEFAULT_STR = "default";
const auto KEY_BINDS = "Up [k] - Down [j]";
const auto MUTED_LABEL = "volume: MUTED";
const auto OUT_LABEL = " Output Devices ";
const auto IN_LABEL = " Input Devices ";

int Jui::connect() {
  if (!pa.init()) {
    printf("failed to init \n");
    return 1;
  }

  if (!pa.connect()) {
    printf("failed to connect \n");
    return 1;
  }
  return 0;
};

void Jui::refresh_state() {
  state.sinks.clear();
  state.sources.clear();
  state.sinks = pa.get_sinks();
  state.sources = pa.get_sources();
  state.info = pa.get_server_info();
};

void Jui::draw() {
  {
    int x{1}, y{2}, width{COLS - 4};

    clear();
    box(stdscr, 0, 0);

    attron(COLOR_PAIR(1));
    mvprintw(0, COLS / 2 - 3, " Jvol ");
    attroff(COLOR_PAIR(1));

    x = draw_keybinds(x, y, show_help);

    mvhline(x, y, 0, width);
    attron(COLOR_PAIR(1));
    auto label = choose_label(selected_tab);
    mvprintw(x, COLS / 2 - strlen(label) / 2, "%s", label);
    attroff(COLOR_PAIR(1));
    x++;

    auto entries = state.sinks;

    switch (selected_tab) {
    case SINKS:
      entries = state.sinks;
      break;
    case SOURCES:
      entries = state.sources;
      break;
    }

    auto sink_n = entries.size();

    for (size_t i = 0; i < sink_n; i++) {
      auto sink = entries[i];
      x = draw_entry_card(x, y, width, sink, i == selected_entry);
      x++;
      mvhline(x, y, 0, width);
      x++;
    }
  };
  refresh();
}

void Jui::handle_key(int key) {
  auto sink_n = state.sinks.size();
  switch (selected_tab) {
  case SINKS:
    sink_n = state.sinks.size();
    break;
  case SOURCES:
    sink_n = state.sources.size();
    break;
  }
  if (key == 'k') {
    selected_entry = std::clamp(selected_entry - 1, 0, (int)sink_n);
  } else if (key == 'j') {
    selected_entry = std::clamp(selected_entry + 1, 0, (int)sink_n - 1);
  } else if (key == 'L') {
    handle_vol(0.01);
  } else if (key == 'H') {
    handle_vol(-0.01);
  } else if (key == 'l') {
    handle_vol(0.1);
  } else if (key == 'h') {
    handle_vol(-0.1);
  } else if (key == 'm') {
    handle_mute();
  } else if (key == 'd') {
    handle_default();
  } else if (key == 'r') {
    refresh_state();
  } else if (key == '?') {
    show_help = !show_help;
  } else if (key == '1') {
    selected_tab = SINKS;
  } else if (key == '2') {
    selected_tab = SOURCES;
  }
};

void Jui::handle_vol(float delta) {
  SinkInfo *s = selected();
  if (s) {
    float vol = get_vol_per(&s->volume) + delta;
    vol = std::clamp(vol, 0.f, 1.f);
    pa.change_volume(s->index, vol, &s->volume, selected_tab == SINKS);
  }
};

void Jui::handle_mute() {
  auto s = selected();
  if (s) {
    pa.change_mute(s->index, s->mute == 1 ? 0 : 1, selected_tab == SINKS);
    refresh_state();
  }
}

void Jui::handle_default() {
  auto s = selected();
  if (s) {
    std::string temp = s->name;
    if (pa.change_default(temp.c_str(), selected_tab == SINKS)) {
      refresh_state();
      switch (selected_tab) {
      case SINKS:
        state.info.default_sink_name = temp;
        break;
      case SOURCES:
        state.info.default_source_name = temp;
        break;
      }
    }
  }
}

SinkInfo *Jui::selected() {
  SinkInfo *s;
  switch (selected_tab) {
  case SINKS:
    s = get_selected(selected_entry, state.sinks);
    break;
  case SOURCES:
    s = get_selected(selected_entry, state.sources);
    break;
  }
  return s;
};

int Jui::draw_entry_card(int x, int y, int width, SinkInfo &sink,
                         bool selected) {
  mvprintw(x, y, "%s", sink.description.c_str());
  if (is_default(sink.name, state, selected_tab)) {
    mvprintw(x, width - strlen(DEFAULT_STR) + 2, "%s", DEFAULT_STR);
  }
  x++;

  if (selected) {
    attron(COLOR_PAIR(2));
  }
  if (sink.mute) {
    mvprintw(x, y, MUTED_LABEL);
  } else {
    mvprintw(x, y, "volume: %.2f%%", get_vol_per(&sink.volume) * 100);
  }
  if (selected) {
    const char *keys = "Vol Up [l] - Vol Down [h]";
    mvprintw(x, width - strlen(keys) + 2, "%s", keys);
    attroff(COLOR_PAIR(2));
  }
  return x;
};

static void draw_ud_key_bids() {
  addstr("Up [");
  attron(COLOR_PAIR(2));
  addch('k');
  attroff(COLOR_PAIR(2));
  addstr("] - Down [");
  attron(COLOR_PAIR(2));
  addch('j');
  attroff(COLOR_PAIR(2));
  addch(']');
}

static void draw_mute_key_bids() {
  addstr("Mute [");
  attron(COLOR_PAIR(2));
  addch('m');
  attroff(COLOR_PAIR(2));
  addch(']');
};

static void draw_def_key_binds() {
  addstr("Set Default [");
  attron(COLOR_PAIR(2));
  addch('d');
  attroff(COLOR_PAIR(2));
  addch(']');
};

static void draw_help_key_binds() {
  addstr("Toggle Help [");
  attron(COLOR_PAIR(2));
  addch('?');
  attroff(COLOR_PAIR(2));
  addch(']');
};

static void draw_refresh_key_binds() {
  addstr("Refresh [");
  attron(COLOR_PAIR(2));
  addch('r');
  attroff(COLOR_PAIR(2));
  addch(']');
};

static void draw_quit_key_binds() {
  addstr("Quit [");
  attron(COLOR_PAIR(2));
  addch('q');
  attroff(COLOR_PAIR(2));
  addch(']');
};

static void draw_vol_keys() {
  addstr("Vol Up 10% [");
  attron(COLOR_PAIR(2));
  addch('l');
  attroff(COLOR_PAIR(2));
  addstr("] - Vol Down 10% [");
  attron(COLOR_PAIR(2));
  addch('h');
  attroff(COLOR_PAIR(2));
  addch(']');
};

static void draw_vol_by_one_keys() {
  addstr("Vol Up 1% [");
  attron(COLOR_PAIR(2));
  addstr("shift + l");
  attroff(COLOR_PAIR(2));
  addstr("] - Vol Down 1% [");
  attron(COLOR_PAIR(2));
  addstr("shift + h");
  attroff(COLOR_PAIR(2));
  addch(']');
};

static void draw_tab_key() {
  addstr("Show Output Devices [");
  attron(COLOR_PAIR(2));
  addch('1');
  attroff(COLOR_PAIR(2));
  addstr("] - Show Input Devices  [");
  attron(COLOR_PAIR(2));
  addch('2');
  attroff(COLOR_PAIR(2));
  addch(']');
};

static bool is_default(std::string name, AppState state, NavTab selected_tab) {
  switch (selected_tab) {
  case SINKS:
    if (name.compare(state.info.default_sink_name) == 0) {
      return true;
    }
    break;
  case SOURCES:
    if (name.compare(state.info.default_source_name) == 0) {
      return true;
    }
    break;
  }
  return false;
}

static int draw_keybinds(int x, int y, bool show_help) {
  move(x, y);
  draw_help_key_binds();
  addstr(" - ");
  draw_quit_key_binds();
  x++;
  move(x, y);
  if (show_help) {
    draw_ud_key_bids();
    x++;
    move(x, y);
    draw_vol_keys();
    x++;
    move(x, y);
    draw_vol_by_one_keys();
    x++;
    move(x, y);
    draw_mute_key_bids();
    addstr(" - ");
    draw_def_key_binds();
    addstr(" - ");
    draw_refresh_key_binds();
    x++;
    move(x, y);
    draw_tab_key();
    x++;
  }
  return x;
};

static const char *choose_label(NavTab tab) {
  switch (tab) {
  case SINKS:
    return OUT_LABEL;
  case SOURCES:
    return IN_LABEL;
  }
  return "";
};
