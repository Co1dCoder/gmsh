// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka.
 

#include "Homology.h"
#include "ChainComplex.h"
#include "OS.h"

#if defined(HAVE_KBIPACK)
Homology::Homology(GModel* model, std::vector<int> physicalDomain, std::vector<int> physicalSubdomain){
  
  _model = model;
  
  Msg::Info("Creating a Cell Complex...");
  double t1 = Cpu();
  
  std::map<int, std::vector<GEntity*> > groups[4];
  model->getPhysicalGroups(groups);
  
  
  std::map<int, std::vector<GEntity*> >::iterator it;
  std::vector<GEntity*> domainEntities;
  std::vector<GEntity*> subdomainEntities;
    
  for(int i = 0; i < physicalDomain.size(); i++){
    for(int j = 0; j < 4; j++){
      it = groups[j].find(physicalDomain.at(i));
      if(it != groups[j].end()){
        std::vector<GEntity*> physicalGroup = (*it).second;
        for(int k = 0; k < physicalGroup.size(); k++){
          domainEntities.push_back(physicalGroup.at(k));
        }
      }
    }
  }
  for(int i = 0; i < physicalSubdomain.size(); i++){           
    for(int j = 0; j < 4; j++){
      it = groups[j].find(physicalSubdomain.at(i));
      if(it != groups[j].end()){
        std::vector<GEntity*> physicalGroup = (*it).second;
        for(int k = 0; k < physicalGroup.size(); k++){
          subdomainEntities.push_back(physicalGroup.at(k));
        }
        
      }
    }
  }
  
  if(domainEntities.empty()) Msg::Warning("Domain is empty.");
  if(subdomainEntities.empty()) Msg::Info("Subdomain is empty.");
  
  
  _cellComplex =  new CellComplex(domainEntities, subdomainEntities);
  
  if(_cellComplex->getSize(0) == 0){ 
    Msg::Error("Cell Complex is empty!");
    Msg::Error("Check the domain & the mesh.");
    return;
  }
  double t2 = Cpu();
  Msg::Info("Cell Complex complete (%g s).", t2 - t1);
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            _cellComplex->getSize(3), _cellComplex->getSize(2), _cellComplex->getSize(1), _cellComplex->getSize(0));
  
}


void Homology::findGenerators(std::string fileName){
  
  Msg::Info("Reducing Cell Complex...");
  double t1 = Cpu();
  int omitted = _cellComplex->reduceComplex(true);
  _cellComplex->combine(3);
  _cellComplex->combine(2);
  _cellComplex->combine(1);
  double t2 = Cpu();
  Msg::Info("Cell Complex reduction complete (%g s).", t2 - t1);
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            _cellComplex->getSize(3), _cellComplex->getSize(2), _cellComplex->getSize(1), _cellComplex->getSize(0));

  _cellComplex->writeComplexMSH(fileName);
  
  Msg::Info("Computing homology groups...");
  t1 = Cpu();
  ChainComplex* chains = new ChainComplex(_cellComplex);
  chains->computeHomology();
  t2 = Cpu();
  Msg::Info("Homology Computation complete (%g s).", t2 - t1);
  
  int HRank[4];
  for(int j = 0; j < 4; j++){
    HRank[j] = 0;
    for(int i = 1; i <= chains->getBasisSize(j); i++){
      
      std::string generator;
      std::string dimension;
      convert(i, generator);
      convert(j, dimension);
      
      std::string name = dimension + "D Generator " + generator;
      Chain* chain = new Chain(_cellComplex->getCells(j), chains->getCoeffVector(j,i), _cellComplex, name, chains->getTorsion(j,i));
      chain->writeChainMSH(fileName);
      if(chain->getSize() != 0) {
        HRank[j] = HRank[j] + 1;
        if(chain->getTorsion() != 1) Msg::Warning("%dD Generator %d has torsion coefficient %d!", j, i, chain->getTorsion());
      }
      delete chain;
    }
  }
  
  Msg::Info("Ranks of homology groups for primal cell complex:");
  Msg::Info("H0 = %d", HRank[0]);
  Msg::Info("H1 = %d", HRank[1]);
  Msg::Info("H2 = %d", HRank[2]);
  Msg::Info("H3 = %d", HRank[3]);
  if(omitted != 0) Msg::Info("Computation of %dD generators was omitted.", _cellComplex->getDim());
  
  
  Msg::Info("Wrote results to %s.", fileName.c_str());
  
  delete chains;
  
  return;
}

void Homology::findThickCuts(std::string fileName){
  
  _cellComplex->removeSubdomain();
  
  Msg::Info("Reducing Cell Complex...");
  double t1 = Cpu();
  int omitted = _cellComplex->coreduceComplex(true);
  _cellComplex->cocombine(0);
  _cellComplex->cocombine(1);
  _cellComplex->cocombine(2);
  double t2 = Cpu();
  Msg::Info("Cell Complex reduction complete (%g s).", t2 - t1);
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            _cellComplex->getSize(3), _cellComplex->getSize(2), _cellComplex->getSize(1), _cellComplex->getSize(0));
 
  _cellComplex->writeComplexMSH(fileName);
  
  Msg::Info("Computing homology groups...");
  t1 = Cpu();
  ChainComplex* chains = new ChainComplex(_cellComplex);
  chains->transposeHMatrices();
  chains->computeHomology(true);
  t2 = Cpu();
  Msg::Info("Homology Computation complete (%g s).", t2- t1);
  
  int dim = _cellComplex->getDim();
  
  int HRank[4];
  for(int i = 0; i < 4; i++) HRank[i] = 0;
  for(int j = 3; j > -1; j--){
    for(int i = 1; i <= chains->getBasisSize(j); i++){
      
      std::string generator;
      std::string dimension;
      convert(i, generator);
      convert(dim-j, dimension);
      
      std::string name = dimension + "D Thick cut " + generator;
      Chain* chain = new Chain(_cellComplex->getCells(j), chains->getCoeffVector(j,i), _cellComplex, name, chains->getTorsion(j,i));
      chain->writeChainMSH(fileName);
      if(chain->getSize() != 0){
        HRank[dim-j] = HRank[dim-j] + 1;
        if(chain->getTorsion() != 1) Msg::Warning("%dD Thick cut %d has torsion coefficient %d!", j, i, chain->getTorsion());
      }
      delete chain;
            
    }
  }
  
  Msg::Info("Ranks of homology groups for dual cell complex:");
  Msg::Info("H0 = %d", HRank[0]);
  Msg::Info("H1 = %d", HRank[1]);
  Msg::Info("H2 = %d", HRank[2]);
  Msg::Info("H3 = %d", HRank[3]);
  if(omitted != 0) Msg::Info("Computation of %dD thick cuts was omitted.", dim);
  
  
  Msg::Info("Wrote results to %s.", fileName.c_str());
  
  delete chains;
  
}
#endif
