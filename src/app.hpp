#ifndef JAPP
#define JAPP

#include "jpulse.hpp"

struct AppState {
  ServerInfo info{};
  std::vector<SinkInfo> sinks{};
  std::vector<SinkInfo> sources{};
};

SinkInfo *get_selected(int i, std::vector<SinkInfo> &sinks);

#endif
