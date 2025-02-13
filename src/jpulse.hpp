#ifndef JPULSE
#define JPULSE

#include <cstdint>
#include <pulse/context.h>
#include <pulse/mainloop.h>
#include <pulse/volume.h>
#include <string>
#include <vector>

const int VOL_MUTE = PA_VOLUME_MUTED;
const int VOL_MAX = PA_VOLUME_NORM;

struct ServerInfo {
  std::string default_sink_name;
  std::string default_source_name;
};

struct SinkInfo {
  std::string name;
  std::string description;
  uint32_t index;
  pa_cvolume volume;
  int mute;
};

class PulseAudio {
public:
  ~PulseAudio();

  bool init();
  bool connect();
  ServerInfo get_server_info();
  std::vector<SinkInfo> get_sinks();
  std::vector<SinkInfo> get_sources();
  bool change_volume(uint32_t index, float vol_percentage, pa_cvolume *vol,
                     bool is_sink);
  bool change_mute(uint32_t index, int mute, bool is_sink);
  bool change_default(const char *name, bool is_sink);

private:
  pa_mainloop *main_loop;
  pa_context *context;
  pa_mainloop_api *api;
  bool run_op(pa_operation *op);
};

float get_vol_per(pa_cvolume *vol);

#endif
