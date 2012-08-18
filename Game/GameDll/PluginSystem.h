#ifndef __PLUGINSYSTEM_H__
#define __PLUGINSYSTEM_H__

#include "IPluginBase.h"
#include "IGameFrameWork.h"

class CPluginSystem
{
public:
    CPluginSystem(void);
    ~CPluginSystem(void);

    bool            Init();
    void            Shutdown();
    IPluginBase*    GetPluginByName(string strName);


private:
    bool            LoadPlugin(string strFilePath);
    std::vector<IPluginBase*> vecPlugins;
};

#endif //__PLUGINSYSTEM_H__

