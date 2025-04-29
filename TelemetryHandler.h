
#include <stdio.h>
#include <mutex>
#include <optional>
#include "irsdk/irsdk_defines.h"
#include "irsdk/irsdk_diskclient.h"


static const int TH_MAX_BUFFERS = 64;

void do_update_telemetry(const std::string path);

void merge_telemetry();

bool init(const std::string path);

void process_telemetry();


static const char* const TelemetryHandlerStr[2][6] = {
	{
		"LFtempL","LFtempM","LFtempR", "RFtempL","RFtempM","RFtempR"
	},
	{
		"LRtempL","LRtempM","LRtempR", "RRtempL","RRtempM","RRtempR"
	}
};

struct TelemetryData {
	float temp[2][6];
};

class TelemetryReader
{
	public:
		bool				init(const std::string path);
		void				processTelemetry(TelemetryData &td);
		void				skipExcessData();
		void				finish();
		
	private:
		bool				m_ready = false;
		int					m_isOnTrack_idx = 0; // bool
		irsdkDiskClient		m_idk;
		int					m_telemetry_data_idx[2][6]; // Front/Rear, Left LMR / Right LMR
		int					m_telemetry_buffer_idx = 0;
		int					m_telemetry_buffer_maxidx = 0;
		TelemetryData		m_telemetry_buffer[TH_MAX_BUFFERS];

};

class TelemetryWriter
{
	public:
		bool init(const std::string path);
		void processTelemetry();
		void finish();
	private:
		irsdkDiskWriter		m_idk;
		bool				m_ready = false;

};

class TelemetryHandler
{
	public:
		TelemetryHandler();
		TelemetryData		getCurrentData();
		void				updateTelemetryFile(const std::string path);
		void				mergeTelemetry();
		void				switchTelemetryReader();
		TelemetryData*		processTelemetry();

	private:
		TelemetryData		m_telemetry_data;
		std::string			m_oldPath;
		std::string			m_telemetryFinalPath;
		std::thread			m_openTelemetryThread;
		std::mutex			m_openTelemetryThread_mtx;

		// The swapping telemetry readers
		TelemetryReader		m_telemetryReader[2];
		bool				m_telemetryReader_cur = 0;
};

extern TelemetryHandler g_telemetryHandler;
