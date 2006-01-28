// $Id: 3D_BGMesh.cpp,v 1.39 2006-01-28 15:02:26 geuzaine Exp $
//
// Copyright (C) 1997-2006 C. Geuzaine, J.-F. Remacle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
// 
// Please report all bugs and problems to <gmsh@geuz.org>.

#include "Gmsh.h"
#include "Mesh.h"
#include "2D_Mesh.h"
#include "3D_Mesh.h"
#include "Views.h"
#include "Numeric.h"
#include "Context.h"

extern Mesh *THEM;
extern Context_T CTX;

static FILE *statfile = NULL;

void ExportStatistics(void *a, void *b)
{
  Element *ele = *(Element**)a;
  if(statfile) ele->ExportStatistics(statfile);
}

void ExportMeshStatistics(Mesh * M, char *filename, int volume, int surface)
{
  if(!Tree_Nbr(M->Volumes) && !Tree_Nbr(M->Surfaces)){
    Msg(GERROR, "No volumes or surfaces to save");
    return;
  }
  else if(volume && !surface && !Tree_Nbr(M->Volumes)){
    Msg(GERROR, "No volumes to save");
    return;
  }
  else if(!volume && surface && !Tree_Nbr(M->Surfaces)){
    Msg(GERROR, "No surfaces to save");
    return;
  }

  statfile = fopen(filename, "w");

  if(!statfile) {
    Msg(GERROR, "Unable to open file '%s'", filename);
    return;
  }

  if(volume && Tree_Nbr(M->Volumes)){
    List_T *l = Tree2List(M->Volumes);
    fprintf(statfile, "View \"Volume Statistics\" {\n");
    fprintf(statfile, "T2(1.e5,30,%d){\"Characteristic Length\", \"Gamma\", \"Eta\", "
	    "\"Rho\", \"Element Number\"};\n", (1<<16)|(4<<8));
    for(int i = 0; i < List_Nbr(l); i++) {
      Volume *vol;
      List_Read(l, i, &vol);
      Tree_Action(vol->Simplexes, ExportStatistics);
      Tree_Action(vol->SimplexesBase, ExportStatistics);
      Tree_Action(vol->Hexahedra, ExportStatistics);
      Tree_Action(vol->Prisms, ExportStatistics);
      Tree_Action(vol->Pyramids, ExportStatistics);
    }
    List_Delete(l);
    fprintf(statfile, "};\n");
  }
  
  if(surface && Tree_Nbr(M->Surfaces)){
    List_T *l = Tree2List(M->Surfaces);
    fprintf(statfile, "View \"Surface Statistics\" {\n");
    fprintf(statfile, "T2(1.e5,30,%d){\"Characteristic Length\", \"Gamma\", \"Eta\", "
	    "\"Rho\", \"Element Number\"};\n", (1<<16)|(4<<8));
    for(int i = 0; i < List_Nbr(l); i++) {
      Surface *surf;
      List_Read(l, i, &surf);
      Tree_Action(surf->Simplexes, ExportStatistics);
      Tree_Action(surf->SimplexesBase, ExportStatistics);
      Tree_Action(surf->Quadrangles, ExportStatistics);
    }
    List_Delete(l);
    fprintf(statfile, "};\n");
  }

  fclose(statfile);
  statfile = NULL;
}

static Mesh *TMPM = NULL;
static double XX, YY, ZZ, DD, LL;

void findcloser(void *a, void *b)
{
  Vertex *v = *(Vertex **) a;
  double d = DSQR(v->Pos.X - XX) + DSQR(v->Pos.Y - YY) + DSQR(v->Pos.Z - ZZ);
  if(d < DD) {
    DD = d;
    LL = v->lc;
  }
}

double Lc_XYZ(double X, double Y, double Z, Mesh * m)
{
  double l;

  switch (m->BGM.Typ) {
  case FUNCTION:
    // for testing...
    l = 0.1 * fabs(cos(2 * 3.14159 * X) * cos( 2 * 3.14159 * Y))  + 0.01;
    break;
  case CONSTANT:
    l = m->BGM.lc;
    break;
  case ONFILE:
    if(Pt_In_Volume(X, Y, Z, TMPM, &l, .01));
    else if(Pt_In_Volume(X, Y, Z, TMPM, &l, .02));
    else if(Pt_In_Volume(X, Y, Z, TMPM, &l, .07));
    else if(Pt_In_Volume(X, Y, Z, TMPM, &l, .1));
    else if(Pt_In_Volume(X, Y, Z, TMPM, &l, .2));
    else if(Pt_In_Volume(X, Y, Z, TMPM, &l, .8));
    else if(Pt_In_Volume(X, Y, Z, TMPM, &l, 20.));
    else {
      XX = X;
      YY = Y;
      ZZ = Z;
      DD = 1.e24;
      LL = 1;
      Tree_Action(TMPM->Vertices, findcloser);
      l = LL;
    }
    break;
  case WITHPOINTS:
    Msg(GERROR, "We should never call Lc_XYZ with BGM.Typ == WITHPOINTS!");
    l = 1.0;
    break;
  }

  if(l <= 0.){
    Msg(WARNING, "Characteristic length <= 0: setting to 1.0");
    l = 1.0;
  }

  return CTX.mesh.lc_factor * l;
}

static void AIG(void *a, void *b)
{
  Simplex *s = *(Simplex **) a;
  AddSimplexInGrid(TMPM, s, BOITE);
}

void BGMWithViewList(int Nb, List_T *List, Tree_T *Pts, int NbNod, Vertex *VertexUp)
{
  if(Nb) {
    int nb = List_Nbr(List) / Nb;
    for(int i = 0; i < List_Nbr(List); i += nb) {
      double *X = (double *)List_Pointer_Fast(List, i);
      double *Y = (double *)List_Pointer_Fast(List, i + NbNod);
      double *Z = (double *)List_Pointer_Fast(List, i + 2 * NbNod);
      double *Val = (double *)List_Pointer_Fast(List, i + 3 * NbNod);
      Vertex *ver[4];
      for(int j = 0; j < NbNod; j++) {
	Vertex V;
        Vertex *v = &V;
        v->Pos.X = X[j];
        v->Pos.Y = Y[j];
        v->Pos.Z = Z[j];
        if(Tree_Query(Pts, &v)) {
          ver[j] = v;
        }
        else {
          v = Create_Vertex(TMPM->MaxPointNum++, X[j], Y[j], Z[j], Val[j], -1.0);
          ver[j] = v;
          Tree_Add(TMPM->Vertices, &v);
          Tree_Add(Pts, &v);
        }
      }
      if(NbNod == 3){ // tri
	Simplex *s = Create_Simplex(ver[0], ver[1], ver[2], VertexUp);
	Tree_Add(TMPM->Simplexes, &s);
      }
      else{
	if(VertexUp){ // quad
	  Simplex *s1 = Create_Simplex(ver[0], ver[1], ver[2], VertexUp);
	  Tree_Add(TMPM->Simplexes, &s1);
	  Simplex *s2 = Create_Simplex(ver[0], ver[2], ver[3], VertexUp);
	  Tree_Add(TMPM->Simplexes, &s2);
	}
	else{ // tet
	  Simplex *s = Create_Simplex(ver[0], ver[1], ver[2], ver[3]);
	  Tree_Add(TMPM->Simplexes, &s);
	}
      }
    }
  }
}

int BGMWithView(Post_View * ErrView)
{
  Vertex *VertexUp;
  Tree_T *Pts;

  if(TMPM){
    Tree_Action(TMPM->Vertices, Free_Vertex);    
    Tree_Action(TMPM->Simplexes, Free_Simplex);    
    Tree_Delete(TMPM->Vertices);
    Tree_Delete(TMPM->Simplexes);
    delete TMPM;
  }

  Pts = Tree_Create(sizeof(Vertex *), comparePosition);
  VertexUp = Create_Vertex(-1, 0., 0., 1., 1., -1.0);

  TMPM = new Mesh;
  TMPM->BGM.Typ = ONFILE;
  TMPM->Vertices = Tree_Create(sizeof(Vertex *), compareVertex);
  TMPM->Simplexes = Tree_Create(sizeof(Simplex *), compareSimplex);
  TMPM->MaxPointNum = 0;
  Create_BgMesh(ONFILE, .2, THEM);

  BGMWithViewList(ErrView->NbST, ErrView->ST, Pts, 3, VertexUp);
  BGMWithViewList(ErrView->NbSQ, ErrView->SQ, Pts, 4, VertexUp);
  BGMWithViewList(ErrView->NbSS, ErrView->SS, Pts, 4, NULL);

  TMPM->Grid.init = 0;
  TMPM->Grid.Nx = 20;
  TMPM->Grid.Ny = 20;
  TMPM->Grid.Nz = 20;
  Tree_Action(TMPM->Vertices, findminmax);
  getminmax(&TMPM->Grid.min.X, &TMPM->Grid.min.Y, &TMPM->Grid.min.Z,
            &TMPM->Grid.max.X, &TMPM->Grid.max.Y, &TMPM->Grid.max.Z);

  if(TMPM->Grid.max.Z == TMPM->Grid.min.Z) {
    TMPM->Grid.Nz = 1;
    Tree_Add(TMPM->Vertices, &VertexUp);
    Tree_Action(TMPM->Vertices, findminmax);
    getminmax(&TMPM->Grid.min.X, &TMPM->Grid.min.Y, &TMPM->Grid.min.Z,
              &TMPM->Grid.max.X, &TMPM->Grid.max.Y, &TMPM->Grid.max.Z);
  }

  Tree_Action(TMPM->Simplexes, AIG);

  Tree_Delete(Pts);

  Msg(INFO, "Background mesh loaded (%d nodes, %d elements)",
      Tree_Nbr(TMPM->Vertices), Tree_Nbr(TMPM->Simplexes));

  return 1;
}
