#ifndef COMCONTROLLER_H
#define COMCONTROLLER_H
#include <Windows.h>
#include <tchar.h>
#include <string>
#include "scssdk_telemetry.h"

class ComController {
	HANDLE m_hComPort;
	LPCTSTR m_comName;
	static const int PACKET_SIZE{ 18 };
	unsigned char m_packet[PACKET_SIZE];
	bool m_comState = false;
	scs_log_t* m_log;

	void readComName();
	std::wstring toWide(char* buffer, int size);

	enum PacketInfo {
		SYNC_FIRST,
		FLAGS_FIRST,
		FLAGS_SECOND,
		GEAR,
		AIR_PRESSURE_WHOLE,
		OIL_PRESSURE_WHOLE,
		OIL_TEMPERATURE,
		WATER_TEMPERATURE,
		SPEED,
		RPM_HIGH,
		RPM_LOW,
		FUEL_INSTANT_CONS_WHOLE,
		FUEL_INSTANT_CONS_FRACTIONAL,
		FUEL_AVG_CONS_WHOLE,
		FUEL_AVG_CONS_FRACTIONAL
	};
	//Прапори першого байту прапорів
	enum FirstByteFlags {
		ELECTRIC_ENABLED = 1,
		ENGINE_ENABLED = 2,
		LEFT_TURN = 4,
		RIGHT_TURN = 8,
		LIGHTS_BEAM_LOW = 16,
		LIGHTS_BEAM_HIGH = 32,
		BATTERY_VOLTAGE_WARNING = 64,
		AIR_PRESSURE_WARNING = 128
	};

	//Прапори другого байту прапорів
	enum SecondByteFlags {
		AIR_PRESSURE_EMERGENCY = 1,
		OIL_PRESSURE_WARNING = 2,
		WATER_TEMPERATURE_WARNING = 4,
		GEAR_SIGN = 8,
		HANDBRAKE = 16,
		CRUISE_ENABLED = 32
	};

public:
	ComController(scs_log_t* log);
	bool openCom();
	void closeCom();
	bool isOpened() { return m_comState; }
	ComController& writeToCom();
	ComController& setSpeed(unsigned char speed);
	ComController& setRPM(int RPM);
	ComController& setAirPressure(float airPressure);
	ComController& setOilPressure(float oilPressure);
	ComController& setWaterTemp(float waterTemp);
	ComController& setOilTemp(float oilTemp);
	ComController& setAvgCons(float avgCons);
	ComController& setInstCons(float instCons);
	ComController& setGear(int gear);
	
	ComController& setElectricEnabled(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | flag; return *this; }
	ComController& setEngineEnabled(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | (flag << 1); return *this; }
	ComController& setLeftBlinker(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | (flag << 2); return *this; }
	ComController& setRightBlinker(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | (flag << 3); return *this; }
	ComController& setLowbeam(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | (flag << 4); return *this; }
	ComController& setHighbeam(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | (flag << 5); return *this; }
	ComController& setVoltageWarning(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | (flag << 6); return *this; }
	ComController& setAirWarning(bool flag) { m_packet[FLAGS_FIRST] = m_packet[FLAGS_FIRST] | (flag << 7); return *this; }
	ComController& setHandbrake(bool flag) { m_packet[FLAGS_SECOND] = m_packet[FLAGS_SECOND] | (flag << 4); return *this; }
	ComController& setCruise(bool flag) { m_packet[FLAGS_SECOND] = m_packet[FLAGS_SECOND] | (flag << 5); return *this; }
	virtual ~ComController();
};

#endif
