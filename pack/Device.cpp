//

#include <cmath>
#include "Device.h"

Device::Device(const pa_source_info *i) {
	type = SOURCE;
	index = i -> index;
	name = i -> name;
	desc = i -> description;
	muted = i -> mute == 1;
	switch (i -> state) {
		case PA_SOURCE_RUNNING:
			state = DEVICE_RUNNING;
			break;
		case PA_SOURCE_IDLE:
			state = DEVICE_IDLE;
			break;
		case PA_SOURCE_SUSPENDED:
			state = DEVICE_SUSPENDED;
			break;
		default:
			state = DEVICE_INVALID_STATE;
	}
	setVolume(&(i -> volume));
}

Device::Device(const pa_sink_info *i) {
	type = SINK;
	index = i -> index;
	name = i -> name;
	desc = i -> description;
	muted = i -> mute == 1;
	switch (i -> state) {
		case PA_SINK_RUNNING:
			state = DEVICE_RUNNING;
			break;
		case PA_SINK_IDLE:
			state = DEVICE_IDLE;
			break;
		case PA_SINK_SUSPENDED:
			state = DEVICE_SUSPENDED;
			break;
		default:
			state = DEVICE_INVALID_STATE;
	}
	setVolume(&(i -> volume));
}

void Device::setVolume(const pa_cvolume *v) {
	volume = *v;
	volume_avg = pa_cvolume_avg(v);
	volume_percentage = (int) round((double)volume_avg * 100. / PA_VOLUME_NORM);
}

Device::Device() {

}
