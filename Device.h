#ifndef AUDIOFFT_DEVICE_H
#define AUDIOFFT_DEVICE_H

#include <string>
#include <pulse/pulseaudio.h>
#include <list>
#include <vector>
#include "Engine.h"

enum DeviceType {
	SOURCE,
	SINK
};

enum DeviceState {
	DEVICE_INVALID_STATE,
	DEVICE_RUNNING,
	DEVICE_IDLE,
	DEVICE_SUSPENDED
};

class Device {
public:
	Device();

	unsigned int index;
	DeviceType type;
	std::string name;
	std::string desc;
	DeviceState state;
	pa_cvolume volume;
	pa_volume_t volume_avg;
	int volume_percentage;
	bool muted;

	explicit Device(const pa_source_info* i);
	explicit Device(const pa_sink_info* i);

private:
	void setVolume(const pa_cvolume* v);
};

static std::vector<Device> used;

typedef enum PulseAudioContextState {
	INIT,
	READY,
	FINISHED
}PulseAudioContextState;

static void stateCallback(pa_context* c, void* userdata) {
	auto* state = static_cast<PulseAudioContextState *>(userdata);
	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_TERMINATED:
			*state = FINISHED;
			break;
		case PA_CONTEXT_READY:
			*state = READY;
			break;
		default:
			break;
	}
}

static void sourceListCallback(pa_context* c, const pa_source_info* i, int eol, void* raw) {
	if (eol != 0) {
		return;
	}

	Device s(i);

	if (i != nullptr && i -> monitor_of_sink != PA_INVALID_INDEX) {
		used.emplace_back(s);
	}
}


#endif //AUDIOFFT_DEVICE_H
