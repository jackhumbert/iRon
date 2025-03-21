
#include <stdio.h>
#include <string>
#include <optional>
#include <thread>
// #include <iostream>
#include "irsdk/irsdk_defines.h"
#include "irsdk/irsdk_diskclient.h"
#include "TelemetryHandler.h"
#include "OverlayDebug.h"
#include "iracing.h"

TelemetryHandler g_telemetryHandler;

/////////////////////////////// Telemetry Reader
void openTelemetry(const std::string path, TelemetryReader *telemetryReader) {
	if (telemetryReader->init(path)) {
		telemetryReader->skipExcessData();
		g_telemetryHandler.switchTelemetryReader();
	}
	else {
		printf("Telemetry read failed! %s\n", path.c_str());
	}
}

bool TelemetryReader::init(const std::string path)
{

	if (m_idk.isFileOpen()) m_idk.closeFile();

	bool done_open = m_idk.openFile(path.c_str());
	// Try again once
	if (!done_open)
	{
		printf("Telemetry read failed! (retrying) %s\n", path.c_str());
		Sleep(2);
		done_open = m_idk.openFile(path.c_str());
	}

	if (done_open)
	{
		m_isOnTrack_idx = m_idk.getVarIdx("IsOnTrack");

		if (m_isOnTrack_idx != -1) {

			const int MAX_STR = 512;
			char tstr[MAX_STR];

			if (1 == m_idk.getSessionStrVal("DriverInfo:DriverCarIdx:", tstr, MAX_STR))
			{
				for (int axle = 0; axle < 2; axle++) {
					for (int sect = 0; sect < 6; sect++) {
						m_telemetry_data_idx[axle][sect] = m_idk.getVarIdx(TelemetryHandlerStr[axle][sect]);
					}
				}
				m_ready = true;
				return true;
			}
		}
	}

	return false;
}

std::optional<TelemetryData> TelemetryReader::processTelemetry()
{
	dbg("Telemetry active: %d", ir_IsDiskLoggingActive.getBool());
	if (!m_ready) return std::nullopt;

	// No more data, leave graph as is
	if (!m_idk.getNextData()) {
		m_idk.closeFile();
		return std::nullopt;
	}

	// isOnTrack indicates that the player is in his car and ready to race
	bool isOnTrack = m_idk.getVarBool(m_isOnTrack_idx);
	

	TelemetryData td;
	// is this a valid data point?
	if (isOnTrack) {
		for (int axle = 0; axle < 2; axle++) {
			for (int sect = 0; sect < 6; sect++) {
				td.temp[axle][sect] = m_idk.getVarFloat(m_telemetry_data_idx[axle][sect]);
			}
		}
		return td;
	}
	
	return std::nullopt;
}

void TelemetryReader::skipExcessData() {
	//printf("Data count: %d\n", m_idk.getDataCount());

	// Only leave the last 64 data points (for a bit of changüí)
	for (int i = 64; i < m_idk.getDataCount(); i++) {
		//printf("%d %d\n", i, m_idk.getNextData());
		m_idk.getNextData();
	}
}

void TelemetryReader::finish() {
	m_ready = false;
	m_idk.closeFile();
}

////////////////////// Telemetry Handler

void TelemetryHandler::updateTelemetryFile(const std::string path) {

	if (!m_oldPath.empty() && path != m_oldPath ) {

		//printf("Opening %s\n", m_oldPath.c_str());
		m_openTelemetryThread = std::thread( openTelemetry, m_oldPath, &m_telemetryReader[!m_telemetryReader_cur]); // !cur, dumbass
		m_openTelemetryThread.detach();

	}

	m_oldPath.assign(path);
	if (m_telemetryFinalPath.empty()) {
		m_telemetryFinalPath.assign(path);
	}
}

TelemetryData TelemetryHandler::processTelemetry() {

	std::optional<TelemetryData> td = m_telemetryReader[m_telemetryReader_cur].processTelemetry();
	// If no new value, return old data
	if (td.has_value()) m_telemetry_data = td.value();
	
	return m_telemetry_data;

}

void TelemetryHandler::mergeTelemetry() {
	if (m_telemetryFinalPath.empty()) return; // No telemetry active

	//printf("Merging telemetry at %s\n", m_telemetryFinalPath.c_str());
	m_telemetryFinalPath.clear();
}

void TelemetryHandler::switchTelemetryReader() {
	m_telemetryReader[m_telemetryReader_cur].finish();
	m_telemetryReader_cur = !m_telemetryReader_cur;
}

TelemetryData TelemetryHandler::getCurrentData() {
	// TODO: Returns a copy, can this be more efficient? How much time does it cost to copy 6 floats?
	return m_telemetry_data;
};
