#include "RfC51GaugePlugin.hpp"          // corresponding header file
#include <math.h>               // for atan2, sqrt
#include <stdio.h>              // for sample output
#include <windows.h>
#include "ser.hpp"


// plugin information
unsigned g_uPluginID          = 2;
char     g_szPluginName[]     = "ForC51Gauge - 2009.06.23";
unsigned g_uPluginVersion     = 002;
unsigned g_uPluginObjectCount = 1;
InternalsPluginInfo g_PluginInfo;


//################  for guage ###############
DWORD lastSendTime=0;
unsigned sendDelay=30;

cnComm com;

//################  for guage end ###############

// interface to plugin information
extern "C" __declspec(dllexport)
const char* __cdecl GetPluginName() { return g_szPluginName; }

extern "C" __declspec(dllexport)
unsigned __cdecl GetPluginVersion() { return g_uPluginVersion; }

extern "C" __declspec(dllexport)
unsigned __cdecl GetPluginObjectCount() { return g_uPluginObjectCount; }

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

const char*    InternalsPluginInfo::GetName()     const { return RfC51GaugePlugin::GetName(); }
const char*    InternalsPluginInfo::GetFullName() const { return m_szFullName; }
const char*    InternalsPluginInfo::GetDesc()     const { return "Example Internals Plugin"; }
const unsigned InternalsPluginInfo::GetType()     const { return RfC51GaugePlugin::GetType(); }
const char*    InternalsPluginInfo::GetSubType()  const { return RfC51GaugePlugin::GetSubType(); }
const unsigned InternalsPluginInfo::GetVersion()  const { return RfC51GaugePlugin::GetVersion(); }
void*          InternalsPluginInfo::Create()      const { return new RfC51GaugePlugin(); }


// InternalsPlugin class

const char RfC51GaugePlugin::m_szName[] = "InternalsPlugin";
const char RfC51GaugePlugin::m_szSubType[] = "Internals";
const unsigned RfC51GaugePlugin::m_uID = 1;
const unsigned RfC51GaugePlugin::m_uVersion = 3;  // set to 3 for InternalsPluginV3 functionality and added graphical and vehicle info


PluginObjectInfo *RfC51GaugePlugin::GetInfo()
{
  return &g_PluginInfo;
}



void RfC51GaugePlugin::UpdateTelemetry( const TelemInfoV2 &info )
{
	DWORD now=GetTickCount();

	if(lastSendTime+sendDelay<now)
	{


	  const unsigned short cusBufferSize = 1024;
	  char* cpTemp = new char[cusBufferSize];

	  // Use the incoming data, for now I'll just write some of it to a file to a) make sure it
	  // is working, and b) explain the coordinate system a little bit (see header for more info)

	  const float metersPerSec = sqrtf( ( info.mLocalVel.x * info.mLocalVel.x ) +
                            ( info.mLocalVel.y * info.mLocalVel.y ) +
                            ( info.mLocalVel.z * info.mLocalVel.z ) );
      char action='S';
      DWORD value=metersPerSec * 3.6f+0.5;
      com.Write(&action,1);
      com.Write(&value,4);

      action='G';
      value=info.mGear+1;  //rf r=-1 ,lfs r=0;
      com.Write(&action,1);
      com.Write(&value,4);


      const float hiddenRpm=info.mEngineMaxRPM*0.4;
      if(info.mEngineRPM>hiddenRpm)
      {
          action='R';
          value=15*((info.mEngineRPM-hiddenRpm)/(info.mEngineMaxRPM-hiddenRpm));
          com.Write(&action,1);
          com.Write(&value,4);
      }else{
          action='R';
          value=0;
          com.Write(&action,1);
        com.Write(&value,4);
      }
      if(info.mEngineRPM>info.mEngineMaxRPM)
      {
          action='C';
          value=1;
          com.Write(&action,1);
          com.Write(&value,4);
      }else{
          action='C';
          value=0;
          com.Write(&action,1);
          com.Write(&value,4);
      }

      sprintf( cpTemp, "[%u]:Speed=%.1fKPH, Gear=%d, Rpm=%.1f ,MaxRPM=%.1f\n", lastSendTime/1000,metersPerSec * 3.6f,info.mGear,info.mEngineRPM,info.mEngineMaxRPM);
      mpConsole->Write(cpTemp);

	  if(cpTemp)delete [] cpTemp;

      lastSendTime=GetTickCount();
	}

}


void RfC51GaugePlugin::Startup()
{
  // Open ports, read configs, whatever you need to do.  For now, I'll just clear out the
  // example output data files.
  const unsigned short cusBufferSize = 1024;
  char* cpTemp = new char[cusBufferSize];
  mpConsole->Write("-STARTUP-\n" );

  DWORD ser_port=GetPrivateProfileInt("config", "port",6,".\\rf_guage_config.ini");
  DWORD dwBaudRate=GetPrivateProfileInt("config", "baudrate",9600,".\\rf_guage_config.ini");
  sendDelay=GetPrivateProfileInt("config","delay",10,".\\rf_guage_config.ini");

  sprintf( cpTemp, "使用串口：%d, 波特率：%d，发送间隔：%d\n", ser_port,dwBaudRate,sendDelay);
  mpConsole->Write(cpTemp);

  com.Open(ser_port,dwBaudRate); //打开串口1并使用默认设置
  if(!com.IsOpen())
  {
    sprintf( cpTemp, "串口 %d 不可用，请检查后重新运行游戏！\n", ser_port);
    mpConsole->Write(cpTemp);
  }
   com.SetBufferSize(1024,1024);

   if(cpTemp)delete [] cpTemp;

}


void RfC51GaugePlugin::Shutdown()
{
  mpConsole->Write("-SHUTDOWN-\n" );
  // Close file
  if( com.IsOpen())
  {
		com.Close();
		mpConsole->Write("关闭串口 !!\n" );
  }
  delete com;
  Sleep(1000);

}


void RfC51GaugePlugin::StartSession()
{
   mpConsole->Write("--STARTSESSION--\n" );
}


void RfC51GaugePlugin::EndSession()
{
  mpConsole->Write("--ENDSESSION--\n" );
}


void RfC51GaugePlugin::EnterRealtime()
{
  mpConsole->Write( "---ENTERREALTIME---\n" );
}


void RfC51GaugePlugin::ExitRealtime()
{
  mpConsole->Write( "---EXITREALTIME---\n" );
}

