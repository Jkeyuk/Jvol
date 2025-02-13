#include "app.hpp"

SinkInfo *get_selected(int i, std::vector<SinkInfo> &sinks) {
  if (i >= 0 && i < sinks.size()) {
    return &sinks[i];
  }
  return NULL;
}
