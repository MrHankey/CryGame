#include "StdAfx.h"
#include "PluginSystem.h"
#include <Windows.h>
#include "CryLibrary.h"
#include "IPluginBase.h"

//define the entry function
typedef IPluginBase*(*TEntryFunction)(ISystem* pSystem);

CPluginSystem::CPluginSystem(void)
{

}


CPluginSystem::~CPluginSystem(void)
{

}

bool CPluginSystem::Init()
{
    
    //Get the full path of the cryengine root
    char workingDirBuffer[1024];
    _getcwd(workingDirBuffer, 1024);

    //Append plugin wildcard
    string strRoot = workingDirBuffer + string("\\Plugins\\");

    if ( GetFileAttributesA(strRoot.c_str()) == INVALID_FILE_ATTRIBUTES )
    {
        
        CryLogAlways("PluginSystem - Plugin folders do not exist. Creating them now...");
        if ( !gEnv->pCryPak->MakeDir( (strRoot + "\\Bin32\\").c_str() ) )
        {
            CryLogAlways("PluginSystem - Failed to create Bin32 folder.");
        }
        if ( !gEnv->pCryPak->MakeDir( (strRoot + "\\Bin64\\").c_str() ) )
        {
            CryLogAlways("PluginSystem - Failed to create Bin64 folder.");
        }
        return false;
    }
    
#ifdef _WIN64
    strRoot += "\\Bin64\\";
#elif _WIN32
    strRoot += "\\Bin32\\";
#endif

    string strFilePattern = strRoot + "*.dll";
    //Find the first match
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    hFind = FindFirstFile(strFilePattern.c_str(), &FindFileData);

    //Log if no plugins were found
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        //CryMessageBox("No plugins found.\n Make sure you placed the plugin DLLs in <cryengineroot>\\Plugins.", "CPluginSystem::Init", MB_OK);
        CryLogAlways("PluginSystem - No Plugins found");
        return false;
    } 
    else 
    {
        do
        {
            LoadPlugin((strRoot + FindFileData.cFileName));
        }
        while (FindNextFile(hFind, &FindFileData) != 0);

        FindClose(hFind);
    }
    return true;
}

IPluginBase* CPluginSystem::GetPluginByName( string strName )
{
    std::vector<IPluginBase*>::iterator pluginIterator;
    for ( pluginIterator = vecPlugins.begin(); pluginIterator != vecPlugins.end(); pluginIterator++ )
    {
        if ( strName.compare( (*pluginIterator)->GetName() ) == 0 )
            return *pluginIterator;
    }

    return NULL;
}

bool CPluginSystem::LoadPlugin( string strFilePath )
{
    string strFileName = PathUtil::GetFileName(strFilePath.c_str());

    //Load the DLL
    HINSTANCE pluginDLL = CryLoadLibrary(strFilePath.c_str());
    if (pluginDLL == NULL)
    {
        CryFatalError("PluginSystem - Failed to load library: %s", strFileName.c_str());
    }

    TEntryFunction InitPluginFunction = (TEntryFunction)CryGetProcAddress(pluginDLL, "Init");
    if (!InitPluginFunction)
    {
        CryFatalError("PluginSystem - Could not find entry function for plugin: %s", strFileName.c_str());
        return false;
    }

    IPluginBase* pPlugin = InitPluginFunction(gEnv->pSystem);

    if (!pPlugin)
    {
        CryFatalError("PluginSystem - Failed to initialize plugin: %s", strFileName.c_str());
        return false;
    }
    if ( GetPluginByName(pPlugin->GetName()) )
    {
        CryFatalError("PluginSystem - Plugin with this name already exists. (%s)", pPlugin->GetName());
        pPlugin->Shutdown();
        return false;
    }

    vecPlugins.push_back(pPlugin);
    CryLogAlways("PluginSystem - Successfully loaded plugin: %s", pPlugin->GetName());

    return true;
}

void CPluginSystem::Shutdown()
{
    std::vector<IPluginBase*>::iterator pluginIterator;
    for ( pluginIterator = vecPlugins.begin(); pluginIterator != vecPlugins.end(); pluginIterator++ )
    {
        (*pluginIterator)->Shutdown();
        //CryFreeLibrary();
    }
}
