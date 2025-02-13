#ifndef JUI
#define JUI

#include "app.hpp"
#include "jpulse.hpp"

enum NavTab { SINKS, SOURCES };

class Jui {

public:
  int connect();
  void draw();
  void handle_key(int key);
  void refresh_state();

private:
  AppState state{};
  PulseAudio pa{};
  int selected_entry{0};
  bool show_help{false};
  NavTab selected_tab{SINKS};
  int draw_entry_card(int x, int y, int width, SinkInfo &sink, bool selected);
  void handle_default();
  void handle_mute();
  void handle_vol(float delta);
  SinkInfo *selected();
};

static const char *choose_label(NavTab tab);
static int draw_keybinds(int x, int y, bool show_help);
static void draw_ud_key_bids();
static void draw_mute_key_bids();
static void draw_def_key_binds();
static void draw_help_key_binds();
static void draw_vol_keys();
static void draw_vol_by_one_keys();
static void draw_refresh_key_binds();
static void draw_quit_key_binds();
static void draw_tab_key();
static bool is_default(std::string name, AppState state, NavTab selected_tab);

#endif
