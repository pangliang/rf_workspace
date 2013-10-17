//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹?
//?                                                                        ?
//?Module: Internals Example Source File                                   ?
//?                                                                        ?
//?Description: Declarations for the Internals Example Plugin              ?
//?                                                                        ?
//?                                                                        ?
//?This source code module, and all information, data, and algorithms      ?
//?associated with it, are part of CUBE technology (tm).                   ?
//?                PROPRIETARY AND CONFIDENTIAL                            ?
//?Copyright (c) 1996-2008 Image Space Incorporated.  All rights reserved. ?
//?                                                                        ?
//?                                                                        ?
//?Change history:                                                         ?
//?  tag.2005.11.30: created                                               ?
//?                                                                        ?
//ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ?
#pragma comment£® lib, "libcurl.dll" £©

#include "Example.hpp"          // corresponding header file
#include <math.h>               // for atan2, sqrt
#include <stdio.h>              // for sample output
#include "include/curl/curl.h"

// plugin information

extern "C" __declspec( dllexport )
const char * __cdecl GetPluginName()                   { return( "ExamplePlugin - 2008.02.13" ); }

extern "C" __declspec( dllexport )
PluginObjectType __cdecl GetPluginType()               { return( PO_INTERNALS ); }

extern "C" __declspec( dllexport )
int __cdecl GetPluginVersion()                         { return( 1 ); } // InternalsPluginV01 functionality

extern "C" __declspec( dllexport )
PluginObject * __cdecl CreatePluginObject()            { return( (PluginObject *) new ExampleInternalsPlugin ); }

extern "C" __declspec( dllexport )
void __cdecl DestroyPluginObject( PluginObject *obj )  { delete( (ExampleInternalsPlugin *) obj ); }


//################  for guage ###############
DWORD lastSaveTime=0;
DWORD saveDelay=500;
char* url=new char[2048];
char* logFilePath=new char[512];
unsigned int BUFFER_SIZE = 51200;
char* buffer = new char[BUFFER_SIZE];

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


void ExampleInternalsPlugin::UpdateScoring( const ScoringInfoV01 &info )
{
  DWORD now=GetTickCount();

    if(now-lastSaveTime>saveDelay && info.mNumVehicles>0)
    {

        memset(buffer,0,BUFFER_SIZE);

        // Print vehicle info
        for( long i = 0; i < info.mNumVehicles; ++i )
        {
            VehicleScoringInfoV01 &vinfo = info.mVehicle[ i ];

            sprintf(buffer, "%s%d,%d,%s,%s,%d,%d,%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n"
                    ,buffer,GetTickCount()/1000,i
                    , vinfo.mDriverName,vinfo.mVehicleClass,vinfo.mIsPlayer, vinfo.mControl
                    , vinfo.mPlace,vinfo.mFinishStatus,vinfo.mNumPitstops,vinfo.mTotalLaps
                    ,vinfo.mBestSector1, vinfo.mBestSector2, vinfo.mBestLapTime
                    ,vinfo.mLastSector1, vinfo.mLastSector2, vinfo.mLastLapTime
                    ,vinfo.mCurSector1, vinfo.mCurSector2);

        }
        saveLog(buffer);
        CreateThread(NULL,NULL, http_post,buffer,0,NULL);

        lastSaveTime=GetTickCount();

    }
}

// ExampleInternalsPlugin class


void ExampleInternalsPlugin::Startup( long version )
{
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


void ExampleInternalsPlugin::Shutdown()
{

}


void ExampleInternalsPlugin::StartSession()
{

}


void ExampleInternalsPlugin::EndSession()
{

}


void ExampleInternalsPlugin::EnterRealtime()
{

}


void ExampleInternalsPlugin::ExitRealtime()
{

}


void ExampleInternalsPlugin::UpdateTelemetry( const TelemInfoV01 &info )
{

}


void ExampleInternalsPlugin::UpdateGraphics( const GraphicsInfoV01 &info )
{

}


bool ExampleInternalsPlugin::CheckHWControl( const char * const controlName, float &fRetVal )
{
  return( false );
}


bool ExampleInternalsPlugin::ForceFeedback( float &forceValue )
{
  // Note that incoming value is the game's computation, in case you're interested.

  // CHANGE COMMENTS TO ENABLE FORCE EXAMPLE
  return( false );

  // I think the bounds are -11500 to 11500 ...
//  forceValue = 11500.0f * sinf( mET );
//  return( true );
}




bool ExampleInternalsPlugin::RequestCommentary( CommentaryRequestInfoV01 &info )
{
  // COMMENT OUT TO ENABLE EXAMPLE
  return( false );
}

