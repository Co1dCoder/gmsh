// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka.

#include "CellComplex.h"

#if defined(HAVE_KBIPACK)

CellComplex::CellComplex( std::vector<GEntity*> domain, std::vector<GEntity*> subdomain ){
  
  _domain = domain;
  _subdomain = subdomain;
  
  // subdomain need to be inserted first!
  insertCells(true);
  insertCells(false);
  
}
void CellComplex::insertCells(bool subdomain){  
  
  std::vector<GEntity*> domain;
  
  if(subdomain) domain = _subdomain;
  else domain = _domain;
  
  std::vector<int> vertices;
  int vertex = 0;
  
  for(unsigned int j=0; j < domain.size(); j++) {  
    for(unsigned int i=0; i < domain.at(j)->getNumMeshElements(); i++){ 
      vertices.clear();
      for(int k=0; k < domain.at(j)->getMeshElement(i)->getNumVertices(); k++){              
        MVertex* vertex = domain.at(j)->getMeshElement(i)->getVertex(k);
        vertices.push_back(vertex->getNum()); 
        //_cells[0].insert(new ZeroSimplex(vertex->getNum(), subdomain, vertex->x(), vertex->y(), vertex->z() )); // Add vertices
        _cells[0].insert(new ZeroSimplex(vertex->getNum(), subdomain));
      } 
      if(domain.at(j)->getMeshElement(i)->getDim() == 3){
        _cells[3].insert(new ThreeSimplex(vertices, subdomain)); // Add volumes
      }
      
      for(int k=0; k < domain.at(j)->getMeshElement(i)->getNumFaces(); k++){
        vertices.clear();
        for(int l=0; l < domain.at(j)->getMeshElement(i)->getFace(k).getNumVertices(); l++){
          vertex = domain.at(j)->getMeshElement(i)->getFace(k).getVertex(l)->getNum();
          vertices.push_back(vertex); 
        } 
        _cells[2].insert(new TwoSimplex(vertices, subdomain)); // Add faces
      }
      
      for(int k=0; k < domain.at(j)->getMeshElement(i)->getNumEdges(); k++){
        vertices.clear();
        for(int l=0; l < domain.at(j)->getMeshElement(i)->getEdge(k).getNumVertices(); l++){
          vertex = domain.at(j)->getMeshElement(i)->getEdge(k).getVertex(l)->getNum();
          vertices.push_back(vertex); 
        }
        _cells[1].insert(new OneSimplex(vertices, subdomain)); // Add edges
      }
              
    }               
  }
  
}

int Simplex::kappa(Cell* tau) const{
  for(int i=0; i < tau->getNumVertices(); i++){
    if( !(this->hasVertex(tau->getVertex(i))) ) return 0;
  }
  
  if(this->getDim() - tau->getDim() != 1) return 0;
  
  int value=1;
  for(int i=0; i < tau->getNumVertices(); i++){
    if(this->getVertex(i) != tau->getVertex(i)) return value;
    value = value*-1;
  }
  
  return value;  
}
std::set<Cell*, Less_Cell>::iterator CellComplex::findCell(int dim, std::vector<int>& vertices){
  if(dim == 0) return _cells[dim].find(new ZeroSimplex(vertices.at(0)));
  if(dim == 1) return _cells[dim].find(new OneSimplex(vertices));
  if(dim == 2) return _cells[dim].find(new TwoSimplex(vertices));
  return _cells[3].find(new ThreeSimplex(vertices));
}

std::set<Cell*, Less_Cell>::iterator CellComplex::findCell(int dim, int vertex, int dummy){
  if(dim == 0) return _cells[dim].lower_bound(new ZeroSimplex(vertex));
  if(dim == 1) return _cells[dim].lower_bound(new OneSimplex(vertex, dummy));
  if(dim == 2) return _cells[dim].lower_bound(new TwoSimplex(vertex, dummy));
  return _cells[3].lower_bound(new ThreeSimplex(vertex, dummy));
}


std::vector<Cell*> CellComplex::bd(Cell* sigma){  
  std::vector<Cell*> boundary;
  int dim = sigma->getDim();
  if(dim < 1) return boundary;
  
  
  citer start = findCell(dim-1, sigma->getVertex(0), 0);
  if(start == lastCell(dim-1)) return boundary;
  
  citer end = findCell(dim-1, sigma->getVertex(sigma->getNumVertices()-1), sigma->getVertex(sigma->getNumVertices()-1));
  if(end != lastCell(dim-1)) end++;

  //for(citer cit = firstCell(dim-1); cit != lastCell(dim-1); cit++){
  for(citer cit = start; cit != end; cit++){
    if(kappa(sigma, *cit) != 0){
      boundary.push_back(*cit);
      if(boundary.size() == sigma->getBdMaxSize()){
        return boundary;
      }
    }
  }
  return boundary;    
}
std::vector< std::set<Cell*, Less_Cell>::iterator > CellComplex::bdIt(Cell* sigma){
  
  int dim = sigma->getDim();  
  std::vector< std::set<Cell*, Less_Cell>::iterator >  boundary;
  if(dim < 1){
    return boundary;
  }
  
  citer start = findCell(dim-1, sigma->getVertex(0), 0);
  if(start == lastCell(dim-1)) return boundary;
  
  citer end = findCell(dim-1, sigma->getVertex(sigma->getNumVertices()-1), sigma->getVertex(sigma->getNumVertices()-1));
  if(end != lastCell(dim-1)) end++;
    
  for(citer cit = start; cit != end; cit++){
    if(kappa(sigma, *cit) != 0){
      boundary.push_back(cit);
      if(boundary.size() == sigma->getBdMaxSize()){
        return boundary;
      }
    }
  }
  
  return boundary;
}



std::vector<Cell*> CellComplex::cbd(Cell* tau){  
  std::vector<Cell*> coboundary;
  int dim = tau->getDim();
  if(dim > 2){
    return coboundary;
  }
  
  for(citer cit = firstCell(dim+1); cit != lastCell(dim+1); cit++){
    if(kappa(*cit, tau) != 0){
      coboundary.push_back(*cit);
      if(coboundary.size() == tau->getCbdMaxSize()){
        return coboundary;
      }
    }
  }
  return coboundary;    
}
std::vector< std::set<Cell*, Less_Cell>::iterator > CellComplex::cbdIt(Cell* tau){
  
  std::vector< std::set<Cell*, Less_Cell>::iterator >  coboundary;
  int dim = tau->getDim();
  if(dim > 2){
    return coboundary;
  }
  
  for(citer cit = firstCell(dim+1); cit != lastCell(dim+1); cit++){
    if(kappa(*cit, tau) != 0){
      coboundary.push_back(cit);
      if(coboundary.size() == tau->getCbdMaxSize()){
        return coboundary;
      }
    }
  }
  
  return coboundary;
}


void CellComplex::removeCell(Cell* cell){
    _cells[cell->getDim()].erase(cell);
  delete cell;
}
void CellComplex::removeCellIt(std::set<Cell*, Less_Cell>::iterator cell){
  Cell* c = *cell;
  int dim = c->getDim();
  _cells[dim].erase(cell);
  delete c;
}

void CellComplex::removeCellQset(Cell*& cell, std::set<Cell*, Less_Cell>& Qset){
   Qset.erase(cell);
  delete cell;
}

void CellComplex::enqueueCellsIt(std::vector< std::set<Cell*, Less_Cell>::iterator >& cells, 
                               std::queue<Cell*>& Q, std::set<Cell*, Less_Cell>& Qset){
  for(unsigned int i=0; i < cells.size(); i++){
    Cell* cell = *cells.at(i);
    citer cit = Qset.find(cell);
    if(cit == Qset.end()){
      Qset.insert(cell);
      Q.push(cell);
    } 
  }
}
void CellComplex::enqueueCells(std::vector<Cell*>& cells, std::queue<Cell*>& Q, std::set<Cell*, Less_Cell>& Qset){
  for(unsigned int i=0; i < cells.size(); i++){
    citer cit = Qset.find(cells.at(i));
    if(cit == Qset.end()){
      Qset.insert(cells.at(i));
      Q.push(cells.at(i));
    }
  }
}


int CellComplex::coreductionMrozek(Cell* generator){
  
  int coreductions = 0;
  
  std::queue<Cell*> Q;
  std::set<Cell*, Less_Cell> Qset;
  
  Q.push(generator);
  Qset.insert(generator);
  //removeCell(generator);
  
  std::vector<Cell*> bd_s;
  //std::vector< std::set<Cell*, Less_Cell>::iterator > bd_s;
  std::vector<Cell*> cbd_c;
  //std::vector< std::set<Cell*, Less_Cell>::iterator > cbd_c;
  Cell* s;
  int round = 0;
  while( !Q.empty() ){
    round++;
    //printf("%d ", round);
    s = Q.front();
    Q.pop();
    removeCellQset(s, Qset);
    
    //bd_s = bdIt(s);
    bd_s = bd(s);
    if( bd_s.size() == 1 && inSameDomain(s, bd_s.at(0)) ){
      removeCell(s);
      //cbd_c = cbdIt(*bd_s.at(0));
      cbd_c = cbd(bd_s.at(0));
      //enqueueCellsIt(cbd_c, Q, Qset);
      enqueueCells(cbd_c, Q, Qset);
      //removeCellIt(bd_s.at(0));
      removeCell(bd_s.at(0));
      coreductions++;
    }
    else if(bd_s.empty()){
      //cbd_c = cbdIt(s);
      //enqueueCellsIt(cbd_c, Q, Qset);
      cbd_c = cbd(s);
      enqueueCells(cbd_c, Q, Qset);
    }
    
  
  }
  printf("Coreduction: %d loops with %d coreductions\n", round, coreductions);
  return coreductions;
}


int CellComplex::coreduction(int dim){
  
  if(dim < 0 || dim > 2) return 0;
  
  std::vector<Cell*> bd_c;
  int count = 0;
  
  bool coreduced = true;
  while (coreduced){
    coreduced = false;
    for(citer cit = firstCell(dim+1); cit != lastCell(dim+1); cit++){
      Cell* cell = *cit;
      
      bd_c = bd(cell);
      if(bd_c.size() == 1 && inSameDomain(cell, bd_c.at(0)) ){
        removeCell(cell);
        removeCell(bd_c.at(0));
        count++;
        coreduced = true;
      }
      
    }
  }
  
  return count;
    
}


int CellComplex::reduction(int dim){
  if(dim < 1 || dim > 3) return 0;
  
  std::vector<Cell*> cbd_c;
  int count = 0;
  
  bool reduced = true;
  while (reduced){
    reduced = false;
    for(citer cit = firstCell(dim-1); cit != lastCell(dim-1); cit++){
      Cell* cell = *cit;
      cbd_c = cbd(cell);
      if(cbd_c.size() == 1 && inSameDomain(cell, cbd_c.at(0)) ){
        removeCell(cell);
        removeCell(cbd_c.at(0));
        count++;
        reduced = true;
      }
      
    }
  }
  return count;  
}

void CellComplex::reduceComplex(){
  printf("Cell complex before reduction: %d volumes, %d faces, %d edges and %d vertices.\n",
         getSize(3), getSize(2), getSize(1), getSize(0));
  reduction(3);
  reduction(2);
  reduction(1);
  printf("Cell complex after reduction: %d volumes, %d faces, %d edges and %d vertices.\n",
         getSize(3), getSize(2), getSize(1), getSize(0));
}
void CellComplex::coreduceComplex(){
  
  std::set<Cell*, Less_Cell> removedCells[4];
  
  printf("Cell complex before coreduction: %d volumes, %d faces, %d edges and %d vertices.\n",
         getSize(3), getSize(2), getSize(1), getSize(0));
  for(int i = 0; i < 3; i++){
    while (getSize(i) != 0){
      citer cit = firstCell(i);
      Cell* cell = *cit;
      removedCells[i].insert(cell);
      _cells[i].erase(cell);
      //removeCell(cell);
      //complex.coreductionMrozek(kaka);
      coreduction(0);
      coreduction(1);
      coreduction(2);
    }
    
  }
  for(int i = 0; i < 3; i++){
    for(citer cit = removedCells[i].begin(); cit != removedCells[i].end(); cit++){
      Cell* cell = *cit;
      _cells[i].insert(cell); 
    }
  }
  printf("Cell complex after coreduction: %d volumes, %d faces, %d edges and %d vertices.\n",
         getSize(3), getSize(2), getSize(1), getSize(0));
}
  
std::vector<gmp_matrix*> CellComplex::constructHMatrices(){
  
  // h_dim: C_dim -> C_(dim-1)
  
  std::vector<gmp_matrix*> HMatrix;

  HMatrix.push_back(NULL);

  for(int dim = 1; dim < 4; dim++){
    unsigned int cols = _cells[dim].size();
    unsigned int rows =  _cells[dim-1].size();
    
    for(citer cit = firstCell(dim); cit != lastCell(dim); cit++){
      Cell* cell = *cit;
      if(cell->inSubdomain()) cols--;
    }
    for(citer cit = firstCell(dim-1); cit != lastCell(dim-1); cit++){
      Cell* cell = *cit;
      if(cell->inSubdomain()) rows--;
    }
    
    
    if( cols == 0 ){
      HMatrix.push_back(NULL);
    }
    else if( rows == 0){
      HMatrix.push_back(create_gmp_matrix_zero(1, cols));
    }
    else{
      long int elems[rows*cols];
      
      citer high = firstCell(dim);
      citer low = firstCell(dim-1);

      unsigned int i = 0;
      while(i < rows*cols){
        while(low != lastCell(dim-1)){      
          Cell* lowcell = *low;
          Cell* highcell = *high;
          if(!(highcell->inSubdomain() || lowcell->inSubdomain())){
            elems[i] = kappa(*high, *low);
            i++;
          }
          low++;
        }
        low = firstCell(dim-1);
        high++;
      }
      HMatrix.push_back(create_gmp_matrix_int(rows, cols, elems));
      
    }
  }
  return HMatrix; 
}


void CellComplex::getDomainVertices(std::set<MVertex*, Less_MVertex>& domainVertices, bool subdomain){

  std::vector<GEntity*> domain;
  if(subdomain) domain = _subdomain;
  else domain = _domain;
  
  for(unsigned int j=0; j < domain.size(); j++) {
    for(unsigned int i=0; i < domain.at(j)->getNumMeshElements(); i++){
      for(int k=0; k < domain.at(j)->getMeshElement(i)->getNumVertices(); k++){
        MVertex* vertex = domain.at(j)->getMeshElement(i)->getVertex(k);
        domainVertices.insert(vertex);
      }
    }
  }
      
/*  for(unsigned int j=0; j < _domain.size(); j++) { 
    for(unsigned int i=0; i < _domain.at(j)->getNumMeshVertices(); i++){
      MVertex* vertex =  _domain.at(j)->getMeshVertex(i);
      domainVertices.insert(vertex);
    }
    
    std::list<GFace*> faces = _domain.at(j)->faces();
    for(std::list<GFace*>::iterator fit = faces.begin(); fit != faces.end(); fit++){
      GFace* face = *fit;
      for(unsigned int i=0; i < face->getNumMeshVertices(); i++){
        MVertex* vertex =  face->getMeshVertex(i);
        domainVertices.insert(vertex);
      }
    }
    std::list<GEdge*> edges = _domain.at(j)->edges();
    for(std::list<GEdge*>::iterator eit = edges.begin(); eit != edges.end(); eit++){
      GEdge* edge = *eit;
      for(unsigned int i=0; i < edge->getNumMeshVertices(); i++){
        MVertex* vertex =  edge->getMeshVertex(i);
        domainVertices.insert(vertex);
      }
    }
    std::list<GVertex*> vertices = _domain.at(j)->vertices();
    for(std::list<GVertex*>::iterator vit = vertices.begin(); vit != vertices.end(); vit++){
      GVertex* vertex = *vit;
      for(unsigned int i=0; i < vertex->getNumMeshVertices(); i++){
        MVertex* mvertex =  vertex->getMeshVertex(i);
        domainVertices.insert(mvertex);
      }
    }
    
  }
 */
  return;
}

int CellComplex::writeComplexMSH(const std::string &name){
  
    
  FILE *fp = fopen(name.c_str(), "w");
  if(!fp){
    Msg::Error("Unable to open file '%s'", name.c_str());
    return 0;
  }
  
  fprintf(fp, "$NOD\n");
  
  std::set<MVertex*, Less_MVertex> domainVertices;
  getDomainVertices(domainVertices, true);
  getDomainVertices(domainVertices, false);
  
  fprintf(fp, "%d\n", domainVertices.size());
  
  for(std::set<MVertex*, Less_MVertex>::iterator vit = domainVertices.begin(); vit != domainVertices.end(); vit++){
    MVertex* vertex = *vit;
    fprintf(fp, "%d %.16g %.16g %.16g\n", vertex->getNum(), vertex->x(), vertex->y(), vertex->z() );
  }
  
      
  fprintf(fp, "$ENDNOD\n");
  fprintf(fp, "$ELM\n");

  fprintf(fp, "%d\n", _cells[0].size() + _cells[1].size() + _cells[2].size() + _cells[3].size());
  
  int index = 1;
  
  for(citer cit = firstCell(0); cit != lastCell(0); cit++) {
    Cell* vertex = *cit;
    fprintf(fp, "%d %d %d %d %d %d\n", index, 15, 0, 1, 1, vertex->getVertex(0));
    index++;
  }
  
  
  for(citer cit = firstCell(1); cit != lastCell(1); cit++) {
    Cell* edge = *cit;
    fprintf(fp, "%d %d %d %d %d %d %d\n", index, 1, 0, 1, 2, edge->getVertex(0), edge->getVertex(1));
    index++;
  }
  
  for(citer cit = firstCell(2); cit != lastCell(2); cit++) {
    Cell* face = *cit;
    fprintf(fp, "%d %d %d %d %d %d %d %d\n", index, 2, 0, 1, 3, face->getVertex(0), face->getVertex(1), face->getVertex(2));
    index++;
  }
  for(citer cit = firstCell(3); cit != lastCell(3); cit++) {
    Cell* volume = *cit;
    fprintf(fp, "%d %d %d %d %d %d %d %d %d\n", index, 4, 0, 1, 4, volume->getVertex(0), volume->getVertex(1), volume->getVertex(2), volume->getVertex(3));
    index++;
  }
    
  fprintf(fp, "$ENDELM\n");
  
  return 1;
}


void CellComplex::printComplex(int dim, bool subcomplex){
  for (citer cit = firstCell(dim); cit != lastCell(dim); cit++){
    Cell* cell = *cit;
    for(int i = 0; i < cell->getNumVertices(); i++){
      if(!subcomplex && !cell->inSubdomain()) printf("%d ", cell->getVertex(i));
    }
    printf("\n");
  }
}

void ChainComplex::KerCod(int dim){
  
  if(dim < 1 || dim > 3 || _HMatrix[dim] == NULL) return;
  
  gmp_matrix* HMatrix = copy_gmp_matrix(_HMatrix[dim], 1, 1, 
                                        gmp_matrix_rows(_HMatrix[dim]), gmp_matrix_cols(_HMatrix[dim]));
  
  gmp_normal_form* normalForm = create_gmp_Hermite_normal_form(HMatrix, NOT_INVERTED, INVERTED);
  //printMatrix(normalForm->left);
  //printMatrix(normalForm->canonical);
  //printMatrix(normalForm->right);
  
  int minRowCol = std::min(gmp_matrix_rows(normalForm->canonical), gmp_matrix_cols(normalForm->canonical));
  int rank = 0;
  mpz_t elem;
  mpz_init(elem);
  
  while(rank < minRowCol){
    gmp_matrix_get_elem(elem, rank+1, rank+1, normalForm->canonical);
    if(mpz_cmp_si(elem,0) == 0) break;
    rank++;
  }
  
  if(rank != gmp_matrix_cols(normalForm->canonical)){
    _kerH[dim] = copy_gmp_matrix(normalForm->right, 1, rank+1, 
                                 gmp_matrix_rows(normalForm->right),  gmp_matrix_cols(normalForm->right));
  }
  
  if(rank > 0){
     _codH[dim] = copy_gmp_matrix(normalForm->canonical, 1, 1,
                                  gmp_matrix_rows(normalForm->canonical), rank);
    gmp_matrix_left_mult(normalForm->left, _codH[dim]);
  }
  
  mpz_clear(elem);
  destroy_gmp_normal_form(normalForm);
  
  return;
}
/*
//j:B->Z
void ChainComplex::Inclusion(int dim){
  
  if(dim < 1 || dim > 3 || _kerH[dim] == NULL || _codH[dim] == NULL) return;
  
  int rows = gmp_matrix_rows(Zbasis);
  int cols = gmp_matrix_cols(Zbasis);
  if(rows < cols) return;
  
  rows = gmp_matrix_rows(Bbasis);
  cols = gmp_matrix_cols(Bbasis);
  if(rows < cols) return;
  
  gmp_matrix* Zbasis = copy_gmp_matrix(_ker[dim], 1, 1,
                                       gmp_matrix_rows(_kerH[dim]), gmp_matrix_cols(_kerH[dim]));
  gmp_matrix* Bbasis = copy_gmp_matrix(_cod[dim], 1, 1,
                                       gmp_matrix_rows(_codH[dim]), gmp_matrix_cols(_codH[dim]));
           
  // A*inv(V) = U*S
  normalForm = create_gmp_Smith_normal_form(Zbasis, NOT_INVERTED, INVERTED);
  
  mpz_t elem;
  mpz_init(elem);
  
  for(int i = 1; i < cols; i++){
    gmp_matrix_get_elem(elem, i, i, normalForm->canonical);
    if(mpz_cmp_si(elem,0) == 0){
      destroy_gmp_normal_form(normalForm);
      return;
    }
  }
  
  gmp_matrix_left_mult(normalForm->left, Bbasis);
  

  
}
*/

void ChainComplex::matrixTest(){
  
  int rows = 3;
  int cols = 6;
  
  long int elems[rows*cols];
  for(int i = 1; i<=rows*cols; i++) elems[i-1] = i;
  
  gmp_matrix* matrix = create_gmp_matrix_int(rows, cols, elems);
  
  gmp_matrix* copymatrix = copy_gmp_matrix(matrix, 3, 2, 3, 5);
  
  printMatrix(matrix);
  printMatrix(copymatrix);
  
  return; 
}


#endif


