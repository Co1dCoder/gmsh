
#ifndef _NODLL
#include <dlfcn.h>
#endif

#include <map>
#include <FL/filename.H>

#include "Plugin.h"
#include "PluginManager.h"

#include "CutPlane.h"
#include "CutSphere.h"
#include "CutMap.h"

using namespace std;

const char *GMSH_PluginEntry = "GMSH_RegisterPlugin";

#if defined(WIN32) && !defined(__CYGWIN__)
#define SLASH "\\"
#else
#define SLASH "/"
#endif


GMSH_PluginManager *GMSH_PluginManager::instance = 0;

GMSH_PluginManager::GMSH_PluginManager()
{
}

GMSH_PluginManager::~GMSH_PluginManager()
{
  for(iter it = allPlugins.begin();
      it != allPlugins.end();
      ++it)delete (*it).second;
}

GMSH_Plugin* GMSH_PluginManager::find (char *pluginName) 
{
  iter it = allPlugins.find(pluginName);
  if ( it == allPlugins.end()) return 0;
  return (*it).second;
}

void GMSH_PluginManager::Action( char *pluginName, char *action, void *data) 
{
  GMSH_Plugin * plugin = find(pluginName);
  if(!plugin)
    {
      throw 1;
    }
  if(!strcmp(action,"Run"))
    {
      plugin->Run();
    }
  else if(!strcmp(action,"Save"))
    {
      plugin->Save();
    }
  else
    {
      throw 1;
    }
}

void GMSH_PluginManager::SetPluginOption (char *pluginName, char *option, double value)
{
  GMSH_Plugin *plugin = find(pluginName);
  if(!plugin)throw "Unknown Plugin Name";
  for(int i=0 ; i<plugin->getNbOptions();i++)
    {
      StringXNumber *sxn;
      // get the ith option of the plugin
      sxn = plugin->GetOption(i);
      // look if it's the good option name
      if(!strcmp (sxn->str,option))
	{
	  sxn->def = value;
	  return;
	}
    }
  throw "Unknown Plugin Option Name";
}

GMSH_PluginManager* GMSH_PluginManager::Instance()
{
  if(!instance)
    {
      instance = new GMSH_PluginManager;
    }
  return instance;
}

void GMSH_PluginManager::RegisterDefaultPlugins()
{
  struct dirent **list;
  char ext[6];

  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("CutPlane" ,GMSH_RegisterCutPlanePlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("CutSphere" ,GMSH_RegisterCutSpherePlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("CutMap" ,GMSH_RegisterCutMapPlugin()));

  char *homeplugins = getenv ("GMSHPLUGINSHOME");
  if(!homeplugins)return;
  int nbFiles = filename_list(homeplugins,&list);
  if(nbFiles <= 0)  return;
  for(int i=0;i<nbFiles;i++)
    {
      char *name = list[i]->d_name;
      if(strlen(name) > 3)
	{
	  strcpy(ext,name+(strlen(name)-3));
	  if(!strcmp(ext,".so") || !strcmp(ext,"dll"))
	  {
	    AddPlugin(homeplugins,name);
	  }
	}
    }
  for(int i=0;i<nbFiles;i++)free(list[i]);
  free (list);
}

void GMSH_PluginManager::AddPlugin( char *dirName, char *pluginName)
{
#if ( defined(WIN32) && !defined(__CYGWIN__) ) || defined(_NODLL)
  Msg(WARNING,"Plugins not yet implemented for WIN32 native compiler");
  return;
#else
  char dynamic_lib[1024];
  char plugin_name[256];
  char plugin_author[256];
  char plugin_copyright[256];
  char plugin_help[256];
  class GMSH_Plugin* (*RegisterPlugin)(void);
  sprintf(dynamic_lib,"%s%s%s",dirName,SLASH,pluginName);
  Msg(INFO,"Opening Plugin %s",dynamic_lib);
  void *hlib = dlopen (dynamic_lib,RTLD_NOW);
  char *err = dlerror();
  if(hlib == NULL)
    {
      Msg(WARNING,"Error in opening %s (dlerror = %s)",dynamic_lib,err);
      return;
    }
  RegisterPlugin = (class GMSH_Plugin* (*)(void)) dlsym(hlib,GMSH_PluginEntry);
  err = dlerror();
  if(err != NULL)
    {
      Msg(WARNING,"Symbol %s missing in Plugin %s (dlerror = %s)",GMSH_PluginEntry,pluginName,err);
      return;
    }

  GMSH_Plugin *p = RegisterPlugin();
  p->hlib = hlib;
  p->getName(plugin_name);
  p->getInfos(plugin_author,plugin_copyright,plugin_help);
  if(allPlugins.find(plugin_name) != allPlugins.end())
    {
      Msg(WARNING,"Plugin %s Multiply defined",pluginName);
      return;
    }
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>(plugin_name,p));
  Msg(INFO,"Plugin name : %s",plugin_name);
  Msg(INFO,"Plugin author : %s",plugin_author);
  Msg(INFO,"Plugin copyright : %s",plugin_copyright);
  Msg(INFO,"Plugin help : %s",plugin_help);
#endif
}



