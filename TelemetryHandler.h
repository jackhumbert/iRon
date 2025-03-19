
#include <stdio.h>
#include "irsdk/irsdk_defines.h"
#include "irsdk/irsdk_diskclient.h"

void do_update_telemetry(const std::string path);

bool init(const std::string path);

void process_telemetry();

static const char* const TelemetryHandlerStr[2][2][3] = {
	{
		{"LFtempL","LFtempM","LFtempR"},
		{"RFtempL","RFtempM","RFtempR"}
	},
	{
		{"LRtempL","LRtempM","LRtempR"},
		{"RRtempL","RRtempM","RRtempR"},
	}
};

extern float g_telemetry_data[2][2][3];
