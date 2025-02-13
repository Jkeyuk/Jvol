#include "jpulse.hpp"
#include <cstdint>
#include <cstdio>
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/mainloop-signal.h>
#include <pulse/volume.h>
#include <sys/types.h>
#include <vector>

static void context_success_cb(pa_context *c, int success, void *userdata) {}

static void source_info_list_builder(pa_context *c, const pa_source_info *i,
                                     int eol, void *userdata) {
  if (i && userdata) {
    std::vector<SinkInfo> *list = (std::vector<SinkInfo> *)userdata;
    list->push_back(
        SinkInfo{i->name, i->description, i->index, i->volume, i->mute});
  }
}

static void sink_info_list_builder(pa_context *c, const pa_sink_info *i,
                                   int eol, void *userdata) {
  if (i && userdata) {
    std::vector<SinkInfo> *list = (std::vector<SinkInfo> *)userdata;
    list->push_back(
        SinkInfo{i->name, i->description, i->index, i->volume, i->mute});
  }
}

static void server_info_callback(pa_context *c, const pa_server_info *i,
                                 void *userdata) {
  if (i && userdata) {
    ServerInfo *info = (ServerInfo *)userdata;
    info->default_sink_name = i->default_sink_name;
    info->default_source_name = i->default_source_name;
  }
}

PulseAudio::~PulseAudio() {
  if (context) {
    pa_context_disconnect(context);
    pa_context_unref(context);
    context = NULL;
  }
  if (main_loop) {
    pa_mainloop_free(main_loop);
    main_loop = NULL;
    api = NULL;
  }
}

bool PulseAudio::init() {
  main_loop = pa_mainloop_new();

  if (!main_loop) {
    printf("loop creation failed \n");
    return false;
  }

  api = pa_mainloop_get_api(main_loop);
  if (pa_signal_init(api) != 0) {
    printf("pa_signal_init fail\n");
    return false;
  }

  context = pa_context_new(api, "jvol");
  if (!context) {
    printf("pa_context_new fail\n");
    return false;
  }

  return true;
}

bool PulseAudio::connect() {
  int err = pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);
  if (err != 0) {
    printf("pa_context_connect fail\n");
    return false;
  }

  bool done = false;
  pa_context_set_state_callback(context, NULL, NULL);

  while (pa_context_state_t s = pa_context_get_state(context)) {
    switch (s) {
    case PA_CONTEXT_CONNECTING:
      pa_mainloop_iterate(main_loop, 1, NULL);
      continue;
    case PA_CONTEXT_AUTHORIZING:
      pa_mainloop_iterate(main_loop, 1, NULL);
      continue;
    case PA_CONTEXT_SETTING_NAME:
      pa_mainloop_iterate(main_loop, 1, NULL);
      continue;
    case PA_CONTEXT_UNCONNECTED:
      pa_mainloop_iterate(main_loop, 1, NULL);
      continue;
    case PA_CONTEXT_READY:
      return true;
    case PA_CONTEXT_FAILED:
      return false;
    case PA_CONTEXT_TERMINATED:
      return false;
    }
  }

  return true;
};

std::vector<SinkInfo> PulseAudio::get_sinks() {
  std::vector<SinkInfo> list{};
  pa_operation *op1 =
      pa_context_get_sink_info_list(context, sink_info_list_builder, &list);
  run_op(op1);
  return list;
};

ServerInfo PulseAudio::get_server_info() {
  ServerInfo info{};
  pa_operation *op =
      pa_context_get_server_info(context, server_info_callback, &info);
  run_op(op);
  return info;
};

bool PulseAudio::change_volume(uint32_t index, float vol_percentage,
                               pa_cvolume *vol, bool is_sink) {
  if (vol_percentage < 0.f || vol_percentage > 1.f) {
    return false;
  }
  uint value = (float)PA_VOLUME_NORM * vol_percentage;
  pa_cvolume_set(vol, vol->channels, value);

  pa_operation *op;
  if (is_sink) {
    op = pa_context_set_sink_volume_by_index(context, index, vol,
                                             context_success_cb, NULL);
  } else {
    op = pa_context_set_source_volume_by_index(context, index, vol,
                                               context_success_cb, NULL);
  }
  return run_op(op);
};

float get_vol_per(pa_cvolume *vol) {
  return (float)pa_cvolume_avg(vol) / (float)PA_VOLUME_NORM;
}

bool PulseAudio::change_mute(uint32_t index, int mute, bool is_sink) {
  pa_operation *op;
  if (is_sink) {
    op = pa_context_set_sink_mute_by_index(context, index, mute,
                                           context_success_cb, NULL);
  } else {
    op = pa_context_set_source_mute_by_index(context, index, mute,
                                             context_success_cb, NULL);
  }
  return run_op(op);
};

bool PulseAudio::change_default(const char *name, bool is_sink) {
  pa_operation *op;
  if (is_sink) {
    op = pa_context_set_default_sink(context, name, context_success_cb, NULL);
  } else {
    op = pa_context_set_default_source(context, name, context_success_cb, NULL);
  }
  return run_op(op);
};

bool PulseAudio::run_op(pa_operation *op) {
  pa_operation_state_t s;
  while ((s = pa_operation_get_state(op)) != PA_OPERATION_DONE) {
    switch (s) {
    case PA_OPERATION_RUNNING:
      pa_mainloop_iterate(main_loop, 1, NULL);
      continue;
    case PA_OPERATION_DONE:
      return true;
    case PA_OPERATION_CANCELLED:
      return false;
    }
  }
  return true;
}

std::vector<SinkInfo> PulseAudio::get_sources() {
  std::vector<SinkInfo> list{};
  pa_operation *op1 =
      pa_context_get_source_info_list(context, source_info_list_builder, &list);
  run_op(op1);
  return list;
};
