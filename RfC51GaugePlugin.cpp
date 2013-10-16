#include "RfC51GaugePlugin.hpp"          // corresponding header file
#include <math.h>               // for atan2, sqrt
#include <stdio.h>              // for sample output
#include <WinSock2.h>
#include <windows.h>



struct OutGaugePack
{
	unsigned	Time;			// time in milliseconds (to check order)

	char		Car[4];			// Car name
	unsigned short		Flags;			// Info (see OG_x below)
	char		Gear;			// Reverse:0, Neutral:1, First:2...
	char		PLID;			// Unique ID of viewed player (0 = none)
	float		Speed;			// M/S
	float		RPM;			// RPM
	float		Turbo;			// BAR
	float		EngTemp;		// C
	float		Fuel;			// 0 to 1
	float		OilPressure;	// BAR
	float		OilTemp;		// C
	unsigned	DashLights;		// Dash lights available (see DL_x below)
	unsigned	ShowLights;		// Dash lights currently switched on
	float		Throttle;		// 0 to 1
	float		Brake;			// 0 to 1
	float		Clutch;			// 0 to 1
	char		Display1[16];	// Usually Fuel
	char		Display2[16];	// Usually Settings

	int			ID;				// optional - only if OutGauge ID is specified
} pack;



// plugin information
unsigned g_uPluginID          = 2;
char     g_szPluginName[]     = "ForC51Gauge - 2009.06.23";
unsigned g_uPluginVersion     = 002;
unsigned g_uPluginObjectCount = 1;
InternalsPluginInfo g_PluginInfo;


//################  for guage ###############
DWORD lastSaveTime=0;
DWORD delay=100;
char* ip=new char[20];
char* car=new char[4];
unsigned port=4444;

unsigned unit=0;

SOCKET socketc;
sockaddr_in addr;
//################  for guage end ###############

// interface to plugin information
extern "C" __declspec(dllexport)
const char* __cdecl GetPluginName()
{
    return g_szPluginName;
}

extern "C" __declspec(dllexport)
unsigned __cdecl GetPluginVersion()
{
    return g_uPluginVersion;
}

extern "C" __declspec(dllexport)
unsigned __cdecl GetPluginObjectCount()
{
    return g_uPluginObjectCount;
}

// get the plugin-info object used to create the plugin.
extern "C" __declspec(dllexport)
PluginObjectInfo* __cdecl GetPluginObjectInfo( const unsigned uIndex )
{
    switch(uIndex)
    {
    case 0:
        return  &g_PluginInfo;
    default:
        return 0;
    }
}


// InternalsPluginInfo class

InternalsPluginInfo::InternalsPluginInfo()
{
    // put together a name for this plugin
    sprintf( m_szFullName, "%s - %s", g_szPluginName, InternalsPluginInfo::GetName() );
}

const char*    InternalsPluginInfo::GetName()     const
{
    return RfC51GaugePlugin::GetName();
}
const char*    InternalsPluginInfo::GetFullName() const
{
    return m_szFullName;
}
const char*    InternalsPluginInfo::GetDesc()     const
{
    return "Example Internals Plugin";
}
const unsigned InternalsPluginInfo::GetType()     const
{
    return RfC51GaugePlugin::GetType();
}
const char*    InternalsPluginInfo::GetSubType()  const
{
    return RfC51GaugePlugin::GetSubType();
}
const unsigned InternalsPluginInfo::GetVersion()  const
{
    return RfC51GaugePlugin::GetVersion();
}
void*          InternalsPluginInfo::Create()      const
{
    return new RfC51GaugePlugin();
}


// InternalsPlugin class

const char RfC51GaugePlugin::m_szName[] = "InternalsPlugin";
const char RfC51GaugePlugin::m_szSubType[] = "Internals";
const unsigned RfC51GaugePlugin::m_uID = 1;
const unsigned RfC51GaugePlugin::m_uVersion = 3;  // set to 3 for InternalsPluginV3 functionality and added graphical and vehicle info

PluginObjectInfo *RfC51GaugePlugin::GetInfo()
{
    return &g_PluginInfo;
}


void RfC51GaugePlugin::UpdateTelemetry(const TelemInfoV2 &info)
{
    DWORD now=GetTickCount();

    if(now-lastSaveTime>delay )
    {

        const unsigned int cusBufferSize = 50*1024;
        char cpTemp[cusBufferSize]={'\0'};

        //pack.Time=now;

        //pack.Flags=0;
        pack.Gear=info.mGear+1;
        //pack.PLID=0;

        const float metersPerSec = sqrtf( ( info.mLocalVel.x * info.mLocalVel.x ) +
                            ( info.mLocalVel.y * info.mLocalVel.y ) +
                            ( info.mLocalVel.z * info.mLocalVel.z ) );
        if(unit==0)
            pack.Speed=metersPerSec * 1.609344;
        else
            pack.Speed=metersPerSec;
        pack.RPM=info.mEngineRPM;
        pack.Fuel=0;
        pack.Clutch=info.mUnfilteredClutch;
        pack.Brake=info.mUnfilteredBrake;
        pack.Throttle=info.mUnfilteredThrottle;
        pack.EngTemp=info.mEngineWaterTemp;
        pack.OilTemp=info.mEngineOilTemp;

        pack.Turbo=10;
        pack.OilPressure=1;


        sendto(socketc,(char*)&pack,sizeof(OutGaugePack),0,(sockaddr*)&addr,sizeof(addr));

        lastSaveTime=GetTickCount();

    }
}



void RfC51GaugePlugin::Startup()
{
    // Open ports, read configs, whatever you need to do.  For now, I'll just clear out the
    // example output data files.

    delay=GetPrivateProfileInt("config", "Delay",1000,".\\outgauge_config.ini");
    GetPrivateProfileString("config", "IP","127.0.0.1",ip,2048,".\\outgauge_config.ini");
    GetPrivateProfileString("config", "Car","FZ5",car,4,".\\outgauge_config.ini");
    port=GetPrivateProfileInt("config", "Port",4444,".\\outgauge_config.ini");
    unit=GetPrivateProfileInt("config", "Unit",0,".\\outgauge_config.ini");
    lastSaveTime=GetTickCount();

    strcpy (pack.Car,car);

    WSADATA wsaData;
    int Ret = WSAStartup(MAKEWORD(2,2),&wsaData);
    printf("Client..!\n");
    if(Ret != 0)
    {
        printf("无法初始化winsock.\n");
        WSACleanup();
    }
    else{

    //    printf("初始化winsock成功\n");
    }

    socketc=::socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);
    addr.sin_addr.S_un.S_addr=inet_addr(ip);
    addr.sin_family=AF_INET;
    addr.sin_port=ntohs(port);
}


void RfC51GaugePlugin::Shutdown()
{

}


void RfC51GaugePlugin::StartSession()
{

}


void RfC51GaugePlugin::EndSession()
{

}


void RfC51GaugePlugin::EnterRealtime()
{

}


void RfC51GaugePlugin::ExitRealtime()
{

}

