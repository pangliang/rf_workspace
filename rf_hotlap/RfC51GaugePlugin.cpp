#include "RfC51GaugePlugin.hpp"          // corresponding header file
#include <math.h>               // for atan2, sqrt
#include <stdio.h>              // for sample output
#include <windows.h>
#include <process.h>
#include "ser.hpp"
#include "include/curl/curl.h"

// plugin information
unsigned g_uPluginID          = 2;
char     g_szPluginName[]     = "ForC51Gauge - 2009.06.23";
unsigned g_uPluginVersion     = 002;
unsigned g_uPluginObjectCount = 1;
InternalsPluginInfo g_PluginInfo;


//################  for guage ###############
DWORD lastSaveTime=0;
DWORD saveDelay=500;
char* url=new char[2048];
char* logFilePath=new char[512];
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



DWORD WINAPI __stdcall http_post(LPVOID lpParameter)
{
    CURL *curl;
    CURLcode res;

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;
    static const char buf[] = "Expect:";

    char* cpTemp=(char*)lpParameter;

    //curl_global_init(CURL_GLOBAL_ALL);

    /* Fill in the submit field too, even if this is rarely needed */
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "data",
                 CURLFORM_COPYCONTENTS, cpTemp,
                 CURLFORM_END);

    curl = curl_easy_init();
    /* initalize custom header list (stating that Expect: 100-continue is not
       wanted */
    headerlist = curl_slist_append(headerlist, buf);
    if(curl)
    {
        /* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* then cleanup the formpost chain */
        curl_formfree(formpost);
        /* free slist */
        curl_slist_free_all (headerlist);
    }

    return 0;
}

void saveLog(const char* cpTemp)
{

    FILE *fo = fopen( logFilePath, "a" );
    if( fo != NULL )
    {
        fprintf( fo, "%s", cpTemp);
        fflush(fo);
        fclose( fo );
    }
}

void RfC51GaugePlugin::UpdateScoring(const ScoringInfoV2 &info)
{

    DWORD now=GetTickCount();

    if(now-lastSaveTime>saveDelay && info.mNumVehicles>0)
    {

        const unsigned int cusBufferSize = 50*1024;
        char cpTemp[cusBufferSize]={'\0'};

        // Print vehicle info
        for( long i = 0; i < info.mNumVehicles; ++i )
        {
            VehicleScoringInfoV2 &vinfo = info.mVehicle[ i ];

            sprintf(cpTemp, "%s%d,%d,%s,%s,%d,%d,%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n"
                    ,cpTemp,GetTickCount()/1000,i
                    , vinfo.mDriverName,vinfo.mVehicleClass,vinfo.mIsPlayer, vinfo.mControl
                    , vinfo.mPlace,vinfo.mFinishStatus,vinfo.mNumPitstops,vinfo.mTotalLaps
                    ,vinfo.mBestSector1, vinfo.mBestSector2, vinfo.mBestLapTime
                    ,vinfo.mLastSector1, vinfo.mLastSector2, vinfo.mLastLapTime
                    ,vinfo.mCurSector1, vinfo.mCurSector2);

        }
        saveLog(cpTemp);
        CreateThread(NULL,NULL, http_post,cpTemp,0,NULL);



        lastSaveTime=GetTickCount();

    }


}



void RfC51GaugePlugin::Startup()
{
    // Open ports, read configs, whatever you need to do.  For now, I'll just clear out the
    // example output data files.

    saveDelay=GetPrivateProfileInt("config", "save_delay",10*1000,".\\rf_guage_config.ini");
    GetPrivateProfileString("config", "url","http://sina.com.cn",url,2048,".\\rf_guage_config.ini");
    GetPrivateProfileString("config", "log_dir","hotlap_data",logFilePath,512,".\\rf_guage_config.ini");
    lastSaveTime=GetTickCount();

    time_t t = time( 0 );
    char tmp[1024];
    strftime( tmp, sizeof(tmp), "%Y%m%d",localtime(&t) );
    sprintf(logFilePath, "%s/%s.txt",logFilePath,tmp);
    if(tmp)
        delete [] tmp;
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

