
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
void openTelemetry(const std::string path, TelemetryReader *telemetryReader, std::mutex *openTelemetryMutex) {
	
	if (!openTelemetryMutex->try_lock()) 
	{
		printf("Mutex not unlocked! Still processing the previous telemetry?\n");
		return;
	}

	if (telemetryReader->init(path)) {
		g_telemetryHandler.switchTelemetryReader();
	}
	else {
		printf("Telemetry read failed! %s\n", path.c_str());
	}

	openTelemetryMutex->unlock();
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

			//const int MAX_STR = 512;
			//char tstr[MAX_STR];

			//if (1 == m_idk.getSessionStrVal("DriverInfo:DriverCarIdx:", tstr, MAX_STR))
			//{
			for (int axle = 0; axle < 2; axle++) {
				for (int sect = 0; sect < 6; sect++) {
					m_telemetry_data_idx[axle][sect] = m_idk.getVarIdx(TelemetryHandlerStr[axle][sect]);
				}
			}

			skipExcessData();

			// Read the last TH_MAX_BUFFERS values into the buffer
			int i;
			for (i = 0; i <= TH_MAX_BUFFERS; i++) {
				if (m_idk.getNextData()) {
					TelemetryData* td = &m_telemetry_buffer[i];
					if (m_idk.getVarBool(m_isOnTrack_idx)) {
						for (int axle = 0; axle < 2; axle++) {
							for (int sect = 0; sect < 6; sect++) {
								td->temp[axle][sect] = m_idk.getVarFloat(m_telemetry_data_idx[axle][sect]);
							}
						}
					}
				}
				else {
					break;
				}
			}
			m_telemetry_buffer_maxidx = i;
			m_idk.closeFile();
			m_ready = true;
			//printf("Loaded %d buffers\n", m_telemetry_buffer_maxidx);
			return true;
		//	}
		}
	}

	return false;
}

void TelemetryReader::processTelemetry(TelemetryData& td)
{
	if (!m_ready) return;

	if (m_telemetry_buffer_idx >= m_telemetry_buffer_maxidx) {
		finish();
		return;
	}

	// Copy 12 floats
	memcpy(&td, &m_telemetry_buffer[m_telemetry_buffer_idx], sizeof(TelemetryData));
	m_telemetry_buffer_idx++;
}

void TelemetryReader::skipExcessData() {
	//printf("Data count: %d\n", m_idk.getDataCount());

	// Only leave the last TH_MAX_BUFFERS data points 
	if (!m_idk.skipData(std::max(0, m_idk.getDataCount() - TH_MAX_BUFFERS))) {
		printf("Error skipping data!");
		finish();
	}
}

void TelemetryReader::finish() {
	m_ready = false;
	m_telemetry_buffer_idx = 0;
	m_idk.closeFile();
}

/////////////////////// Telemetry Writer

bool TelemetryWriter::init(const std::string path) {
	return true;
}

////////////////////// Telemetry Handler

TelemetryHandler::TelemetryHandler() {
	TelemetryData m_telemetry_data;
}

void TelemetryHandler::updateTelemetryFile(const std::string path) {

	// When the first telemetry filename comes in, skip and save the filename.
	// When the second telemetry filename comes in, we process the first one.

	// FIXME: If you don't close the overlay between sessions, it will open the Last-session-Last-telemetry file for one second instead of starting at 0.0
	
	if (!m_oldPath.empty() && path != m_oldPath ) {

		//printf("Opening %s\n", m_oldPath.c_str());
		m_openTelemetryThread = std::thread(openTelemetry, m_oldPath, &m_telemetryReader[!m_telemetryReader_cur], &m_openTelemetryThread_mtx); // !cur, dumbass
		m_openTelemetryThread.detach();

	}

	m_oldPath.assign(path);
	if (m_telemetryFinalPath.empty()) {
		m_telemetryFinalPath.assign(path);
	}
}

TelemetryData* TelemetryHandler::processTelemetry() {

	m_telemetryReader[m_telemetryReader_cur].processTelemetry(m_telemetry_data);
	// If no new value, return old data
	//if (td.has_value()) m_telemetry_data = td.value();
	
	return &m_telemetry_data;

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
