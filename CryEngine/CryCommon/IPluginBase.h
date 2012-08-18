#ifndef __IPLUGINBASE_H__
#define __IPLUGINBASE_H__

class IPluginBase
{
public:
    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual const char* GetName() = 0;
};

#endif// __IPLUGINBASE_H__