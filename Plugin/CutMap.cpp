// $Id: CutMap.cpp,v 1.16 2001-08-06 10:35:47 geuzaine Exp $

#include "CutMap.h"
#include "List.h"

StringXNumber CutMapOptions_Number[] = {
  { GMSH_FULLRC, "A" , NULL , 1. },
  { GMSH_FULLRC, "iView" , NULL , -1. }
};

extern "C"
{
  GMSH_Plugin *GMSH_RegisterCutMapPlugin ()
  {
    return new GMSH_CutMapPlugin (CutMapOptions_Number[0].def,
				  (int)CutMapOptions_Number[1].def);
  }
}


GMSH_CutMapPlugin::GMSH_CutMapPlugin(double a, int I)
  :A(a),iView(I)
{
}

void GMSH_CutMapPlugin::getName(char *name) const
{
  strcpy(name,"Cut Map");
}

void GMSH_CutMapPlugin::getInfos(char *author, char *copyright, char *help_text) const
{
  strcpy(author, "J.-F. Remacle (remacle@scorec.rpi.edu)");
  strcpy(copyright, "DGR (www.multiphysics.com)");
  strcpy(help_text, 
	 "Extracts the isovalue surface of value A from a\n"
	 "3D scalar map. Script name: Plugin(CutMap).");
}

int GMSH_CutMapPlugin::getNbOptions() const
{
  return sizeof(CutMapOptions_Number)/sizeof(StringXNumber);
}

StringXNumber *GMSH_CutMapPlugin:: GetOption (int iopt)
{
  return &CutMapOptions_Number[iopt];
}

void GMSH_CutMapPlugin::CatchErrorMessage (char *errorMessage) const
{
  strcpy(errorMessage,"CutMap failed...");
}

double GMSH_CutMapPlugin :: levelset (double x, double y, double z, double val) const
{
  // we must look into the map for A - Map(x,y,z)
  // this is the case when the map is the same as the view,
  // the result is the extraction of isovalue A
  return A - val;
}

extern List_T *Post_ViewList;

Post_View *GMSH_CutMapPlugin::execute (Post_View *v)
{
  Post_View *vv;
  A = CutMapOptions_Number[0].def;
  iView = (int)CutMapOptions_Number[1].def;
  
  if(v && iView < 0)
    vv = v;
  else if(!(vv = (Post_View*)List_Pointer_Test(Post_ViewList,iView))){
    Msg(WARNING,"Plugin CutMap: View[%d] does not exist",iView);
    return 0;
  }

  return GMSH_LevelsetPlugin::execute(vv);
}

void GMSH_CutMapPlugin::Run () 
{ 
  int iView = (int)CutMapOptions_Number[1].def;
  if(iView < 0) CutMapOptions_Number[1].def = 0;
  execute (0);
}
