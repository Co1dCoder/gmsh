// $Id: Read_Mesh.cpp,v 1.73 2004-05-25 23:16:27 geuzaine Exp $
//
// Copyright (C) 1997-2004 C. Geuzaine, J.-F. Remacle
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
#include "Geo.h"
#include "CAD.h"
#include "Mesh.h"
#include "3D_Mesh.h"
#include "Create.h"
#include "MinMax.h"
#include "Context.h"

extern Context_T CTX;

// Read mesh in native MSH format

#define LGN1 1
#define TRI1 2
#define QUA1 3
#define TET1 4
#define HEX1 5
#define PRI1 6
#define PYR1 7
#define LGN2 8
#define TRI2 9
#define QUA2 10
#define TET2 11
#define HEX2 12
#define PRI2 13
#define PYR2 14
#define PNT  15

#define NB_NOD_MAX_ELM 30

void addPhysicalGroup(Mesh * M, int Type, int Physical, int Elementary)
{
  PhysicalGroup *pg;
  if((pg = FindPhysicalGroup(Physical, Type, M))) {
    List_Insert(pg->Entities, &Elementary, fcmp_int);
  }
  else {
    List_T *tmp = List_Create(1, 1, sizeof(int));
    List_Add(tmp, &Elementary);
    pg = Create_PhysicalGroup(Physical, Type, tmp);
    List_Add(M->PhysicalGroups, &pg);
    List_Delete(tmp);
  }
}

/* Note: the "Dirty" flag only has an influence if one doesn't load
   the geometry along with the mesh (since we only add "dirty"
   geometrical entities if they don't already exist). */

Curve *addElementaryCurve(Mesh * M, int Num)
{
  Curve *c;
  if(!(c = FindCurve(Num, M))) {
    c = Create_Curve(Num, MSH_SEGM_LINE, 0, NULL, NULL, -1, -1, 0., 1.);
    c->Dirty = 1;
    Tree_Add(M->Curves, &c);
  }
  return c;
}

Surface *addElementarySurface(Mesh * M, int Num)
{
  Surface *s;
  if(!(s = FindSurface(Num, M))) {
    s = Create_Surface(Num, MSH_SURF_PLAN);
    s->Dirty = 1;
    Tree_Add(M->Surfaces, &s);
  }
  return s;
}

Volume *addElementaryVolume(Mesh * M, int Num)
{
  Volume *v;
  if(!(v = FindVolume(Num, M))) {
    v = Create_Volume(Num, MSH_VOLUME);
    v->Dirty = 1;
    Tree_Add(M->Volumes, &v);
  }
  return v;
}

void Read_Mesh_MSH(Mesh * M, FILE * fp)
{
  char String[256];
  double version = 1.0;
  int format = LIST_FORMAT_ASCII, size = sizeof(double);
  int Nbr_Nodes, Nbr_Elements, i_Node, i_Element;
  int Num, Type, Physical, Elementary, Partition, i, j;
  int NbTags, Tag;
  double x, y, z, lc1, lc2;
  Vertex *vert, verts[NB_NOD_MAX_ELM], *vertsp[NB_NOD_MAX_ELM], **vertspp;
  Simplex *simp;
  Quadrangle *quad;
  Hexahedron *hex;
  Prism *pri;
  Pyramid *pyr;
  Curve *c;
  Surface *s;
  Volume *v;
  Tree_T *Duplicates = NULL;

  while(1) {
    do {
      fgets(String, sizeof(String), fp);
      if(feof(fp))
        break;
    } while(String[0] != '$');

    if(feof(fp))
      break;

    /*  F o r m a t  */

    if(!strncmp(&String[1], "MeshFormat", 10)) {
      fscanf(fp, "%lf %d %d\n", &version, &format, &size);

      if(version == 2.0){
	Msg(INFO, "Detected mesh file format %g", version);
      }
      else{
	Msg(GERROR, "Unknown MSH file version to read (%g)", version);
	return;
      }

      if(format == 0)
        format = LIST_FORMAT_ASCII;
      else if(format == 1)
        format = LIST_FORMAT_BINARY;
      else {
        Msg(GERROR, "Unknown data format for mesh");
        return;
      }
    }

    /*  POINTS -- this field is deprecated, and will eventually disappear */

    else if(!strncmp(&String[1], "PTS", 3) ||
	    !strncmp(&String[1], "Points", 6)) {

      fscanf(fp, "%d", &Nbr_Nodes);
      Msg(INFO, "%d points", Nbr_Nodes);

      for(i_Node = 0; i_Node < Nbr_Nodes; i_Node++) {
        fscanf(fp, "%d %lf %lf %lf %lf %lf", &Num, &x, &y, &z, &lc1,
               &lc2);
        vert = Create_Vertex(Num, x, y, z, lc1, lc2);
        if(!Tree_Insert(M->Points, &vert)){
	  Msg(GERROR, "Point %d already exists", vert->Num);
	  Free_Vertex(&vert, 0);
	}
      }
    }

    /*  NODES  */

    else if(!strncmp(&String[1], "NOD", 3) ||
	    !strncmp(&String[1], "NOE", 3) ||
	    !strncmp(&String[1], "Nodes", 5)) {

      fscanf(fp, "%d", &Nbr_Nodes);
      Msg(INFO, "%d nodes", Nbr_Nodes);

      if(CTX.mesh.check_duplicates)
        Duplicates = Tree_Create(sizeof(Vertex *), comparePosition);
      for(i_Node = 0; i_Node < Nbr_Nodes; i_Node++) {
        fscanf(fp, "%d %lf %lf %lf", &Num, &x, &y, &z);
        vert = Create_Vertex(Num, x, y, z, 1.0, 0.0);
        if(!Tree_Insert(M->Vertices, &vert)){
	  Msg(GERROR, "Node %d already exists", vert->Num);
	  Free_Vertex(&vert, 0);
	}
	else if(CTX.mesh.check_duplicates) {
          if((vertspp = (Vertex **) Tree_PQuery(Duplicates, &vert)))
            Msg(GERROR, "Nodes %d and %d have identical coordinates (%g, %g, %g)",
                Num, (*vertspp)->Num, x, y, z);
          else
            Tree_Add(Duplicates, &vert);
        }
      }
      if(CTX.mesh.check_duplicates)
        Tree_Delete(Duplicates);
    }

    /* ELEMENTS */

    else if(!strncmp(&String[1], "ELM", 3) ||
	    !strncmp(&String[1], "Elements", 8)) {

      fscanf(fp, "%d", &Nbr_Elements);
      Msg(INFO, "%d elements", Nbr_Elements);

      if(CTX.mesh.check_duplicates)
        Duplicates = Tree_Create(sizeof(Vertex *), comparePosition);

      for(i_Element = 0; i_Element < Nbr_Elements; i_Element++) {
	
	if(version <= 1.0){
	  fscanf(fp, "%d %d %d %d %d",
		 &Num, &Type, &Physical, &Elementary, &Nbr_Nodes);
	  Partition = Physical;
	}
	else{
	  fscanf(fp, "%d %d %d", &Num, &Type, &NbTags);
	  Elementary = Physical = Partition = 1;
	  for(j = 0; j < NbTags; j++){
	    fscanf(fp, "%d", &Tag);	    
	    if(j == 0)
	      Physical = Tag;
	    else if(j == 1)
	      Elementary = Tag;
	    else if(j == 2)
	      Partition = Tag;
	    // ignore any other tags for now
	  }
	  switch (Type) {
	  case PNT : Nbr_Nodes = 1; break;
	  case LGN1: Nbr_Nodes = 2; break;
	  case LGN2: Nbr_Nodes = 3; break;
	  case TRI1: Nbr_Nodes = 3; break;
	  case TRI2: Nbr_Nodes = 6; break;
	  case QUA1: Nbr_Nodes = 4; break;
	  case QUA2: Nbr_Nodes = 8 + 1; break;
	  case TET1: Nbr_Nodes = 4; break;
	  case TET2: Nbr_Nodes = 10; break;
	  case HEX1: Nbr_Nodes = 8; break;
	  case HEX2: Nbr_Nodes = 20 + 6; break;
	  case PRI1: Nbr_Nodes = 6; break;
	  case PRI2: Nbr_Nodes = 15 + 3; break;
	  case PYR1: Nbr_Nodes = 5; break;
	  case PYR2: Nbr_Nodes = 13 + 1; break;
	  }
	}

        for(j = 0; j < Nbr_Nodes; j++)
          fscanf(fp, "%d", &verts[j].Num);
	
        for(i = 0; i < Nbr_Nodes; i++) {
          vertsp[i] = &verts[i];
          if(!(vertspp = (Vertex **) Tree_PQuery(M->Vertices, &vertsp[i])))
            Msg(GERROR, "Unknown vertex %d in element %d", verts[i].Num, Num);
          else
            vertsp[i] = *vertspp;
        }

        if(CTX.mesh.check_duplicates) {
          vert = Create_Vertex(Num, 0., 0., 0., 1.0, 0.0);
          for(i = 0; i < Nbr_Nodes; i++) {
            vert->Pos.X += vertsp[i]->Pos.X;
            vert->Pos.Y += vertsp[i]->Pos.Y;
            vert->Pos.Z += vertsp[i]->Pos.Z;
          }
          vert->Pos.X /= (double)Nbr_Nodes;
          vert->Pos.Y /= (double)Nbr_Nodes;
          vert->Pos.Z /= (double)Nbr_Nodes;
          if((vertspp = (Vertex **) Tree_PQuery(Duplicates, &vert)))
            Msg(WARNING, "Elements %d and %d have identical barycenters",
                Num, (*vertspp)->Num);
          else
            Tree_Add(Duplicates, &vert);
        }

        switch (Type) {
        case LGN1:
        case LGN2:
	  c = addElementaryCurve(M, abs(Elementary));
	  addPhysicalGroup(M, MSH_PHYSICAL_LINE, Physical, abs(Elementary));
          simp = Create_Simplex(vertsp[0], vertsp[1], NULL, NULL);
          simp->Num = Num;
          simp->iEnt = Elementary;
          simp->iPart = Add_MeshPartition(Partition, M);
	  if(Type == LGN2){
	    simp->VSUP = (Vertex **) Malloc(1 * sizeof(Vertex *));
	    simp->VSUP[0] = vertsp[2];
	    simp->VSUP[0]->Degree = 2;
	  }
          if(!Tree_Insert(c->Simplexes, &simp)){
	    Msg(GERROR, "Line element %d already exists", simp->Num);
	    Free_Simplex(&simp, 0);
	  }
          break;
        case TRI1:
        case TRI2:
	  s = addElementarySurface(M, Elementary);
	  addPhysicalGroup(M, MSH_PHYSICAL_SURFACE, Physical, Elementary);
          simp = Create_Simplex(vertsp[0], vertsp[1], vertsp[2], NULL);
          simp->Num = Num;
          simp->iEnt = Elementary;
          simp->iPart = Add_MeshPartition(Partition, M);
	  if(Type == TRI2){
	    simp->VSUP = (Vertex **) Malloc(3 * sizeof(Vertex *));
	    for(i = 0; i < 3; i++){
	      simp->VSUP[i] = vertsp[i+3];
	      simp->VSUP[i]->Degree = 2;
	    }
	  }
          if(!Tree_Insert(s->Simplexes, &simp)){
	    Msg(GERROR, "Triangle %d already exists", simp->Num);
	    Free_Simplex(&simp, 0);
	  }
          break;
        case QUA1:
        case QUA2:
	  s = addElementarySurface(M, Elementary);
	  addPhysicalGroup(M, MSH_PHYSICAL_SURFACE, Physical, Elementary);
          quad = Create_Quadrangle(vertsp[0], vertsp[1], vertsp[2], vertsp[3]);
          quad->Num = Num;
          quad->iEnt = Elementary;
          quad->iPart = Add_MeshPartition(Partition, M);
	  if(Type == QUA2){
	    quad->VSUP = (Vertex **) Malloc((4 + 1) * sizeof(Vertex *));
	    for(i = 0; i < 4 + 1; i++){
	      quad->VSUP[i] = vertsp[i+4];
	      quad->VSUP[i]->Degree = 2;
	    }
	  }
          if(!Tree_Insert(s->Quadrangles, &quad)){
	    Msg(GERROR, "Quadrangle %d already exists", simp->Num);
	    Free_Simplex(&simp, 0);
	  }
          break;
        case TET1:
        case TET2:
	  v = addElementaryVolume(M, Elementary);
	  addPhysicalGroup(M, MSH_PHYSICAL_VOLUME, Physical, Elementary);
          simp = Create_Simplex(vertsp[0], vertsp[1], vertsp[2], vertsp[3]);
          simp->Num = Num;
          simp->iEnt = Elementary;
          simp->iPart = Add_MeshPartition(Partition, M);
	  if(Type == TET2){
	    simp->VSUP = (Vertex **) Malloc(6 * sizeof(Vertex *));
	    for(i = 0; i < 6; i++){
	      simp->VSUP[i] = vertsp[i+4];
	      simp->VSUP[i]->Degree = 2;
	    }
	  }
          if(!Tree_Insert(v->Simplexes, &simp)){
	    Msg(GERROR, "Tetrahedron %d already exists", simp->Num);
	    Free_Simplex(&simp, 0);
	  }
          break;
        case HEX1:
        case HEX2:
	  v = addElementaryVolume(M, Elementary);
	  addPhysicalGroup(M, MSH_PHYSICAL_VOLUME, Physical, Elementary);
          hex = Create_Hexahedron(vertsp[0], vertsp[1], vertsp[2], vertsp[3],
                                  vertsp[4], vertsp[5], vertsp[6], vertsp[7]);
          hex->Num = Num;
          hex->iEnt = Elementary;
          hex->iPart = Add_MeshPartition(Partition, M);
	  if(Type == HEX2){
	    hex->VSUP = (Vertex **) Malloc((12 + 6) * sizeof(Vertex *));
	    for(i = 0; i < 12 + 6; i++){
	      hex->VSUP[i] = vertsp[i+8];
	      hex->VSUP[i]->Degree = 2;
	    }
	  }
          if(!Tree_Insert(v->Hexahedra, &hex)){
	    Msg(GERROR, "Hexahedron %d already exists", hex->Num);
	    Free_Hexahedron(&hex, 0);
	  }
          break;
        case PRI1:
        case PRI2:
	  v = addElementaryVolume(M, Elementary);
	  addPhysicalGroup(M, MSH_PHYSICAL_VOLUME, Physical, Elementary);
          pri = Create_Prism(vertsp[0], vertsp[1], vertsp[2],
                             vertsp[3], vertsp[4], vertsp[5]);
          pri->Num = Num;
          pri->iEnt = Elementary;
          pri->iPart = Add_MeshPartition(Partition, M);
	  if(Type == PRI2){
	    pri->VSUP = (Vertex **) Malloc((9 + 3) * sizeof(Vertex *));
	    for(i = 0; i < 9 + 3; i++){
	      pri->VSUP[i] = vertsp[i+6];
	      pri->VSUP[i]->Degree = 2;
	    }
	  }
          if(!Tree_Insert(v->Prisms, &pri)){
	    Msg(GERROR, "Prism %d already exists", pri->Num);
	    Free_Prism(&pri, 0);
	  }
          break;
        case PYR1:
        case PYR2:
	  v = addElementaryVolume(M, Elementary);
	  addPhysicalGroup(M, MSH_PHYSICAL_VOLUME, Physical, Elementary);
          pyr = Create_Pyramid(vertsp[0], vertsp[1], vertsp[2],
                               vertsp[3], vertsp[4]);
          pyr->Num = Num;
          pyr->iEnt = Elementary;
          pyr->iPart = Add_MeshPartition(Partition, M);
	  if(Type == PYR2){
	    pyr->VSUP = (Vertex **) Malloc((8 + 1) * sizeof(Vertex *));
	    for(i = 0; i < 8 + 1; i++){
	      pyr->VSUP[i] = vertsp[i+5];
	      pyr->VSUP[i]->Degree = 2;
	    }
	  }
          if(!Tree_Insert(v->Pyramids, &pyr)){
	    Msg(GERROR, "Pyramid %d already exists", pri->Num);
	    Free_Pyramid(&pyr, 0);
	  }
          break;
        case PNT:
	  addPhysicalGroup(M, MSH_PHYSICAL_POINT, Physical, Elementary);
	  // we need to make a new one: vertices in M->Vertices and
	  // M->Points should never point to the same memory location
	  vert = Create_Vertex(vertsp[0]->Num, vertsp[0]->Pos.X, vertsp[0]->Pos.Y, 
			       vertsp[0]->Pos.Z, vertsp[0]->lc, vertsp[0]->w);
          if(!Tree_Insert(M->Points, &vert)){
	    Msg(WARNING, "Point %d already exists", vert->Num);
	    Free_Vertex(&vert, 0);
	  }
          break;
        default:
          Msg(WARNING, "Unknown type of element in Read_Mesh");
          break;
        }
      }

      if(CTX.mesh.check_duplicates) {
        Tree_Action(Duplicates, Free_Vertex);
        Tree_Delete(Duplicates);
      }

    }

    do {
      fgets(String, 256, fp);
      if(feof(fp))
        Msg(GERROR, "Prematured end of mesh file");
    } while(String[0] != '$');

  }

  if(Tree_Nbr(M->Volumes))
    M->status = 3;
  else if(Tree_Nbr(M->Surfaces))
    M->status = 2;
  else if(Tree_Nbr(M->Curves))
    M->status = 1;
  else if(Tree_Nbr(M->Points))
    M->status = 0;
  else
    M->status = -1;

  // For efficiency reasons, we store the partition index (and not the
  // partition number) in the various mesh elements. We need to
  // re-sort the list according to these indices to allow direct
  // access through List_Pointer & co.
  List_Sort(M->Partitions, compareMeshPartitionIndex);
}

// Read mesh in VTK format

void Read_Mesh_VTK(Mesh * m, FILE *fp)
{
  char line[256], dumline1[256], dumline2[256];
  int i;
  int NbFaces, NbVertices, Vertex1, Vertex2, Vertex3, NbVerticesOnFace;
  double x, y, z;
  Vertex *v1, *v2, *v3;

  fgets(line, 255, fp);
  fgets(line, 255, fp);
  fgets(line, 255, fp);
  fgets(line, 255, fp);
  fgets(line, 255, fp);

  sscanf(line, "%s %d %s", dumline1, &NbVertices, dumline2);
  Surface *surf = Create_Surface(1, MSH_SURF_DISCRETE);
  surf->Dirty = 1;
  Tree_Add(m->Surfaces, &surf);
  for(i = 0; i < NbVertices; i++) {
    fscanf(fp, "%le %le %le", &x, &y, &z);
    Vertex *v = Create_Vertex(i, x, y, z, 1.0, 1.0);
    Tree_Add(m->Vertices, &v);
    Tree_Add(surf->Vertices, &v);
    v->ListSurf = List_Create(1, 1, sizeof(Surface *));
    List_Add(v->ListSurf, &surf);
  }

  fscanf(fp, "%s %d %d", dumline1, &NbFaces, &i);
  for(int i = 0; i < NbFaces; i++) {
    fscanf(fp, "%d", &NbVerticesOnFace);
    Simplex *s;
    if(NbVerticesOnFace == 3) {
      fscanf(fp, "%d %d %d", &Vertex1, &Vertex2, &Vertex3);
      v1 = FindVertex(Vertex1, m);
      v2 = FindVertex(Vertex2, m);
      v3 = FindVertex(Vertex3, m);
      if(!v1 || !v2 || !v3){
	Msg(GERROR, "Bad vertex reference in VTK file: aborting");
	return;
      }
      else{
	s = Create_Simplex(v1, v2, v3, NULL);
	s->Num = i;
	s->iEnt = 1;
      }
    }
    if(!(surf = FindSurface(1, m))) {
      surf = Create_Surface(1, MSH_SURF_DISCRETE);
      surf->Dirty = 1;
      Tree_Add(m->Surfaces, &surf);
    }
    Tree_Add(surf->Simplexes, &s);
  }

  if(NbFaces)
    m->status = 2;

  Volume *vol = Create_Volume(1, MSH_VOLUME);
  vol->Dirty = 1;
  vol->Surfaces = List_Create(1, 1, sizeof(Surface *));
  List_Add(vol->Surfaces, &surf);
  Tree_Add(m->Volumes, &vol);
}

// Read mesh in SMS format

#define ENTITY_VERTEX 0
#define ENTITY_EDGE   1
#define ENTITY_FACE   2
#define ENTITY_REGION 3
#define ENTITY_NONE   4

void Read_Mesh_SMS(Mesh * m, FILE * in)
{
  char line[1023];
  int i, patch, nbPts;
  int NbRegions, NbFaces, NbEdges, NbVertices, NbPoints,
    GEntityType, GEntityId, EntityNbConnections, Dummy,
    Edge1, Edge2, Edge3, Edge4, Face1, Face2, Face3, Face4;
  int VertexId1, VertexId2, NbEdgesOnFace, NbFacesOnRegion;
  double x, y, z, u, v;
  List_T *AllEdges, *AllFaces;
  Vertex *v1 = NULL, *v2 = NULL, *v3 = NULL, *v4 = NULL;

  fscanf(in, "%s %d", line, &Dummy);
  fscanf(in, "%d %d %d %d %d", &NbRegions, &NbFaces, &NbEdges, &NbVertices,
         &NbPoints);

  Msg(INFO, "Reading a mesh in scorec format");
  Msg(INFO, "%d Vertices", NbVertices);

  for(i = 0; i < NbVertices; i++) {
    fscanf(in, "%d", &GEntityId);
    if(GEntityId) {
      fscanf(in, "%d %d %lf %lf %lf", &GEntityType, &EntityNbConnections, &x,
             &y, &z);
      Vertex *vert = Create_Vertex(i, x, y, z, 1.0, 1.0);
      Tree_Add(m->Vertices, &vert);
      switch (GEntityType) {
      case 0:
	{
	  // we need to make a new one: vertices in m->Vertices and
	  // m->Points should never point to the same memory location
	  Vertex *pnt = Create_Vertex(i, x, y, z, 1.0, 1.0);
	  Tree_Add(m->Points, &pnt);
	}
        break;
      case 1:
        fscanf(in, "%le", &u);
        break;
      case 2:
        fscanf(in, "%le %le %d", &u, &v, &patch);
        break;
      case 3:
        break;
      }
    }
  }

  Msg(INFO, "%d edges", NbEdges);
  AllEdges = List_Create(NbEdges, 1, sizeof(Edge));
  Edge e;

  for(int i = 0; i < NbEdges; i++) {
    fscanf(in, "%d", &GEntityId);

    if(GEntityId) {
      fscanf(in, "%d %d %d %d %d", &GEntityType, &VertexId1, &VertexId2,
             &EntityNbConnections, &nbPts);
      for(int j = 0; j < nbPts; j++) {
        switch (GEntityType) {
        case 0:
          break;
        case 1:
          fscanf(in, "%le", &u);
          break;
        case 2:
          fscanf(in, "%le %le %d", &u, &v, &patch);
          break;
        case 3:
          break;
        }
      }
      e.Points = NULL;
      Vertex *v1 = FindVertex(VertexId1 - 1, m);
      Vertex *v2 = FindVertex(VertexId2 - 1, m);
      e.V[0] = v1;
      e.V[1] = v2;
      List_Add(AllEdges, &e);
      switch (GEntityType) {
      case ENTITY_EDGE:
        Simplex * s = Create_Simplex(v1, v2, NULL, NULL);
        Curve *c;
        if(!(c = FindCurve(GEntityId, m))) {
          c = Create_Curve(GEntityId, MSH_SEGM_DISCRETE, 1, NULL, NULL, -1, -1, 0, 1);
	  c->Dirty = 1;
          Tree_Add(m->Curves, &c);
        }
        s->iEnt = GEntityId;
        s->Num = i;
        Tree_Add(c->Simplexes, &s);
      }
    }
  }

  AllFaces = List_Create(NbFaces, 1, sizeof(Simplex *));

  Volume *vol = Create_Volume(1, MSH_VOLUME);
  vol->Dirty = 1;
  vol->Surfaces = List_Create(1, 1, sizeof(Surface *));
  Tree_Add(m->Volumes, &vol);

  Msg(INFO, "%d faces", NbFaces);
  for(int i = 0; i < NbFaces; i++) {
    fscanf(in, "%d", &GEntityId);
    if(GEntityId) {
      fscanf(in, "%d %d", &GEntityType, &NbEdgesOnFace);

      List_T *Lists[4] = { 0, 0, 0, 0 };

      if(NbEdgesOnFace == 3) {
        fscanf(in, "%d %d %d %d", &Edge1, &Edge2, &Edge3, &nbPts);
        List_Read(AllEdges, abs(Edge1) - 1, &e);
        Lists[0] = e.Points;
        if(Edge1 > 0)
          v1 = e.V[0];
        else
          v1 = e.V[1];
        List_Read(AllEdges, abs(Edge2) - 1, &e);
        Lists[1] = e.Points;
        if(Edge2 > 0)
          v2 = e.V[0];
        else
          v2 = e.V[1];
        List_Read(AllEdges, abs(Edge3) - 1, &e);
        Lists[2] = e.Points;
        if(Edge3 > 0)
          v3 = e.V[0];
        else
          v3 = e.V[1];
        v4 = NULL;
      }
      else if(NbEdgesOnFace == 4) {
        fscanf(in, "%d %d %d %d %d", &Edge1, &Edge2, &Edge3, &Edge4, &nbPts);
        List_Read(AllEdges, abs(Edge1) - 1, &e);
        if(Edge1 > 0)
          v1 = e.V[0];
        else
          v1 = e.V[1];
        List_Read(AllEdges, abs(Edge2) - 1, &e);
        if(Edge2 > 0)
          v2 = e.V[0];
        else
          v2 = e.V[1];
        List_Read(AllEdges, abs(Edge3) - 1, &e);
        if(Edge3 > 0)
          v3 = e.V[0];
        else
          v3 = e.V[1];
        List_Read(AllEdges, abs(Edge4) - 1, &e);
        if(Edge4 > 0)
          v4 = e.V[0];
        else
          v4 = e.V[1];
      }
      else {
        Msg(GERROR, "Wrong number pf edges on face (%d)", NbEdgesOnFace);
      }
      for(int j = 0; j < nbPts; j++) {
        switch (GEntityType) {
        case 0:
          break;
        case 1:
          fscanf(in, "%le", &u);
          break;
        case 2:
          fscanf(in, "%le %le %d", &u, &v, &patch);
          break;
        case 3:
          break;
        }
      }

      Simplex *s = Create_Simplex(v1, v2, v3, v4);
      s->Num = i + 1;
      s->iEnt = GEntityId + 10000;

      Surface *surf;
      List_Add(AllFaces, &s);

      switch (GEntityType) {
      case ENTITY_REGION:
        break;
      case ENTITY_FACE:
        if(!(surf = FindSurface(GEntityId + 10000, m))) {
          surf = Create_Surface(GEntityId + 10000, MSH_SURF_DISCRETE);
          surf->Dirty = 1;
          if(!NbRegions)
            List_Add(vol->Surfaces, &surf);
          Tree_Add(m->Surfaces, &surf);
        }
        Tree_Add(surf->Vertices, &s->V[0]);
        Tree_Add(surf->Vertices, &s->V[1]);
        Tree_Add(surf->Vertices, &s->V[2]);
        Tree_Add(surf->Simplexes, &s);
      }
    }
  }

  Msg(INFO, "%d region", NbRegions);

  for(int i = 0; i < NbRegions; i++) {
    fscanf(in, "%d", &GEntityId);
    if(GEntityId) {
      fscanf(in, "%d", &NbFacesOnRegion);
      Simplex *myS1, *myS2;
      if(NbFacesOnRegion == 4) {
        fscanf(in, "%d %d %d %d %d", &Face1, &Face2, &Face3, &Face4, &Dummy);
        List_Read(AllFaces, abs(Face1) - 1, &myS1);
        List_Read(AllFaces, abs(Face2) - 1, &myS2);
        v1 = myS1->V[0];
        v2 = myS1->V[1];
        v3 = myS1->V[2];
        for(int hh = 0; hh < 3; hh++)
          if(compareVertex(&v1, &myS2->V[hh]) &&
             compareVertex(&v2, &myS2->V[hh]) &&
             compareVertex(&v3, &myS2->V[hh]))
            v4 = myS2->V[hh];
      }
      if(!v1 || !v2 || !v3 || !v4) {
        Msg(GERROR, "%d\n", NbFacesOnRegion);
        Msg(GERROR, "%p %p %p %p\n", v1, v2, v3, v4);
	Msg(GERROR, "%p %p %p \n", myS1->V[0], myS1->V[1], myS1->V[2]);
        Msg(GERROR, "%p %p %p \n", myS2->V[0], myS2->V[1], myS2->V[2]);
        return;
      }
      Simplex *s = Create_Simplex(v1, v2, v3, v4);

      if(!(vol = FindVolume(GEntityId, m))) {
        vol = Create_Volume(GEntityId, MSH_VOLUME);
	vol->Dirty = 1;
        Tree_Add(m->Volumes, &vol);
      }
      s->iEnt = GEntityId;
      Tree_Insert(vol->Simplexes, &s);
      Tree_Insert(m->Simplexes, &s);
    }
  }

  List_Delete(AllEdges);
  List_Delete(AllFaces);

  if(Tree_Nbr(m->Volumes)) {
    m->status = 3;
  }
  else if(Tree_Nbr(m->Surfaces)) {
    m->status = 2;
  }
  else if(Tree_Nbr(m->Curves)) {
    m->status = 1;
  }
  else if(Tree_Nbr(m->Points))
    m->status = 0;
  else
    m->status = -1;
}

// Public Read_Mesh routine

void Read_Mesh(Mesh * M, FILE * fp, char *filename, int type)
{
  if(filename)
    Msg(INFO, "Reading mesh file '%s'", filename);

  switch (type) {
  case FORMAT_MSH: Read_Mesh_MSH(M, fp); break;
  case FORMAT_SMS: Read_Mesh_SMS(M, fp); break;
  case FORMAT_VTK: Read_Mesh_VTK(M, fp); break;
  default:
    Msg(GERROR, "Unkown mesh file format");
    return;
  }

  if(filename){
    Msg(INFO, "Read mesh file '%s'", filename);
    Msg(STATUS2N, "Read '%s'", filename);
  }
}
