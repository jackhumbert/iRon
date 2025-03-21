
#include <stdio.h>
#include <optional>
#include "irsdk/irsdk_defines.h"
#include "irsdk/irsdk_diskclient.h"

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
		std::optional<TelemetryData>	processTelemetry();
		void				skipExcessData();
		void				finish();
		
	private:
		irsdkDiskClient		m_idk;
		int					m_telemetry_data_idx[2][6]; // Front/Rear, Left LMR / Right LMR
		int					m_isOnTrack_idx = 0; // bool
		int					m_ready = false;
};

class TelemetryHandler
{
	public:
		TelemetryData		getCurrentData();
		void				updateTelemetryFile(const std::string path);
		void				mergeTelemetry();
		void				switchTelemetryReader();
		TelemetryData		processTelemetry();

	private:
		TelemetryData		m_telemetry_data;
		std::string			m_oldPath;
		std::string			m_telemetryFinalPath;
		std::thread			m_openTelemetryThread;

		TelemetryReader		m_telemetryReader[2];
		bool				m_telemetryReader_cur = 0;
};

extern TelemetryHandler g_telemetryHandler;
