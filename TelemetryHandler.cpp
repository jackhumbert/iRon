
#include <stdio.h>
#include <string>
// #include <iostream>
#include "irsdk/irsdk_defines.h"
#include "irsdk/irsdk_diskclient.h"
#include "TelemetryHandler.h"

irsdkDiskClient idk;
float g_telemetry_data[2][2][3]; // Front/Rear, Left/Right, Left/Middle/Right
int telemetry_data_idx[2][2][3]; // Front/Rear, Left/Right, Left/Middle/Right
int isOnTrack_idx = 0; // bool 

std::string oldPath;

void do_update_telemetry(const std::string path) {

	if (!oldPath.empty()) {

		printf("Opening %s\n", oldPath.c_str());
		if (init(oldPath)) {
			printf("Data count: %d\n", idk.getDataCount());
			
			// Only leave the last 64 data points
			for (int i = 64; i < idk.getDataCount(); i++) {
				//printf("%d %d\n", i, idk.getNextData());
				idk.getNextData();
			}

			//shutdown();
		}
		else {
			printf("Telemetry read failed!\n");
		}

	}

	oldPath.assign(path);
}

bool init(const std::string path)
{

	if (idk.isFileOpen()) idk.closeFile();

	if (idk.openFile(path.c_str()))
	{
		isOnTrack_idx = idk.getVarIdx("IsOnTrack");
		for (int axle = 0; axle <= 1; axle++) {
			for (int side = 0; side <= 1; side++) {
				for (int sect = 0; sect <= 2; sect++) {
					telemetry_data_idx[axle][side][sect] = idk.getVarIdx(TelemetryHandlerStr[axle][side][sect]);
				}
			}
		}

		if (isOnTrack_idx != -1) {

			const int MAX_STR = 512;
			char tstr[MAX_STR];

			if (1 == idk.getSessionStrVal("DriverInfo:DriverCarIdx:", tstr, MAX_STR))
			{
				return true;
			}
		}
	}

	return false;
}

void process_telemetry()
{
	// No more data, leave graph as is
	if (!idk.getNextData()) {
		idk.closeFile();
		return;
	}

	// isOnTrack indicates that the player is in his car and ready to race
	bool isOnTrack = idk.getVarBool(isOnTrack_idx);

	// is this a valid data point?
	if (isOnTrack) {
		for (int axle = 0; axle <= 1; axle++) {
			for (int side = 0; side <= 1; side++) {
				for (int sect = 0; sect <= 2; sect++) {
					g_telemetry_data[axle][side][sect] = idk.getVarFloat(telemetry_data_idx[axle][side][sect]);
				}
			}
		}
	}
}
