#include "ComController.h"
#include <iostream>
#include <cassert>

ComController::ComController(scs_log_t* log) :
    m_comName{ L"COM6" },
    m_hComPort{ NULL },
    m_log{log}
{
    //readComName();
    memset(m_packet, 0, PACKET_SIZE);
}

bool ComController::openCom()
{
    m_hComPort = CreateFile(m_comName, GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hComPort == INVALID_HANDLE_VALUE) {
        m_comState = false;
        return m_comState;
    }
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);
    if (!GetCommState(m_hComPort, &serialParams)) {
        m_comState = false;
        return m_comState;
    }
    serialParams.BaudRate = CBR_115200;
    serialParams.ByteSize = 8;
    serialParams.Parity = NOPARITY;
    serialParams.StopBits = ONESTOPBIT;
    assert("Cannot set COM state" && SetCommState(m_hComPort, &serialParams));
    if (!SetCommState(m_hComPort, &serialParams)) {
        m_comState = false;
        return m_comState;
    }
    m_packet[SYNC_FIRST] = 126;
    m_comState = true;
    return m_comState;
}

void ComController::closeCom()
{
    CloseHandle(m_hComPort);
}

ComController& ComController::writeToCom()
{
    DWORD dwBytesWritten;
    bool isWritten = WriteFile(m_hComPort, m_packet, PACKET_SIZE, &dwBytesWritten, NULL);
    assert("Cannot write to COM" && isWritten);
    assert("Not all bytes are written" && PACKET_SIZE == dwBytesWritten);
    //(*m_log)(SCS_LOG_TYPE_message, "Writing to COM finished");
    memset(m_packet, 0, PACKET_SIZE);
    m_packet[SYNC_FIRST] = 126;
    return *this;
}

ComController& ComController::setSpeed(unsigned char speed)
{
    if (speed > 160) speed = 160;
    else if (speed < 0) speed = 0;
    //---------����������� �������---------
    if (speed <= 50);
    else if (speed <= 60) speed *= 1.016;
    else if (speed <= 70) speed *= 1.028;
    else if (speed <= 80) speed *= 1.025;
    else if (speed <= 90) speed *= 1.044;
    else if (speed <= 100) speed *= 1.04;
    else if (speed <= 110) speed *= 1.036;
    else if (speed <= 120) speed *= 1.033;
    else if (speed <= 130) speed *= 1.023;
    else if (speed <= 140) speed *= 1.014;
    else if (speed <= 150) speed *= 1.013;
    //-------------------------------------
    m_packet[SPEED] = speed;
    return *this;
}

ComController& ComController::setRPM(int RPM)
{
    if (RPM > 400) RPM = 400;
    else if (RPM < 0) RPM = 0;
    //---------����������� �������---------
    if (RPM <= 150) RPM *= 0.93333;
    else if (RPM <= 200) RPM *= 0.96;
    else if (RPM <= 250) RPM *= 0.98;
    else if (RPM <= 300);
    else if (RPM <= 350) RPM *= 0.99;
    else RPM *= 0.987;
    //-------------------------------------
    m_packet[RPM_HIGH] = RPM >> 8;
    m_packet[RPM_LOW] = RPM & 0b11111111;
    return *this;
}

ComController& ComController::setAirPressure(float airPressure)
{
    if (airPressure > 150) airPressure = 150;
    else if (airPressure < 0) airPressure = 0;
    m_packet[AIR_PRESSURE_WHOLE] = floor(airPressure);
    return *this;
}

ComController& ComController::setOilPressure(float oilPressure)
{
    if (oilPressure > 70) oilPressure = 70;
    else if (oilPressure < 0) oilPressure = 0;
    m_packet[OIL_PRESSURE_WHOLE] = floor(oilPressure);
    return *this;
}

ComController& ComController::setWaterTemp(float waterTemp)
{
    if (waterTemp > 255) waterTemp = 255;
    else if (waterTemp < 0) waterTemp = 0;
    m_packet[WATER_TEMPERATURE] = waterTemp;
    return *this;
}

ComController& ComController::setOilTemp(float oilTemp)
{
    if (oilTemp > 255) oilTemp = 255;
    else if (oilTemp < 0) oilTemp = 0;
    m_packet[OIL_TEMPERATURE] = oilTemp;
    return *this;
}

ComController& ComController::setAvgCons(float avgCons)
{
    unsigned char wholePart = floor(avgCons);
    unsigned char fractionalPart = floor(avgCons * 10) - wholePart * 10;
    m_packet[FUEL_AVG_CONS_WHOLE] = wholePart;
    m_packet[FUEL_AVG_CONS_FRACTIONAL] = fractionalPart;
    return *this;
}

ComController& ComController::setInstCons(float instCons)
{
    unsigned char wholePart = floor(instCons);
    unsigned char fractionalPart = floor(instCons * 10) - wholePart * 10;
    m_packet[FUEL_INSTANT_CONS_WHOLE] = wholePart;
    m_packet[FUEL_INSTANT_CONS_FRACTIONAL] = fractionalPart;
    return *this;
}

ComController& ComController::setGear(int gear)
{
    m_packet[FLAGS_SECOND] = m_packet[FLAGS_SECOND] | ((gear > 0) ? 0 : 1 << 3);
    if (gear < 0) gear *= -1;
    m_packet[GEAR] = gear;
    return *this;
}

ComController::~ComController()
{
    CloseHandle(m_hComPort);
}

void ComController::readComName()
{
    LPCTSTR filename{ L"conf.txt" };
    HANDLE hFile = CreateFile(filename, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    const DWORD dwBytesToRead{ 12 };
    DWORD dwBytesReaded{ 0 };
    char buffer[dwBytesToRead + 1];
    assert("File not found" && hFile != INVALID_HANDLE_VALUE);
    assert("Cannot read from file" && ReadFile(hFile, buffer, dwBytesToRead, &dwBytesReaded, NULL));
    assert("File is not completed" && dwBytesReaded == dwBytesToRead);
    buffer[12] = '\0';
    std::wstring tmp = toWide(buffer + 8, 4).substr(0, 4);
    m_comName = L"COM6";
    CloseHandle(hFile);
}

std::wstring ComController::toWide(char* buffer, int size)
{
    int len;
    int slength = size;
    len = MultiByteToWideChar(CP_ACP, NULL, buffer, slength, NULL, NULL);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, NULL, buffer, slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
