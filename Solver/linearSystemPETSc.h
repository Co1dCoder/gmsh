// Gmsh - Copyright (C) 1997-2013 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.

#ifndef _LINEAR_SYSTEM_PETSC_H_
#define _LINEAR_SYSTEM_PETSC_H_

// Interface to PETSc 3.x

// Options for PETSc can be provided on the command line, or in the
// file ~/.petscrc. By default, we use the following options (GMRES
// with ILU(6) preconditioner and RCMK renumbering):
//
//   -ksp_type gmres
//   -pc_type ilu
//   -pc_factor_levels 6
//   -pc_factor_mat_ordering rcm
//   -ksp_rtol 1.e-8
//
// Other useful options include:
//
//   -ksp_gmres_restart 100
//   -ksp_monitor
//
// To use a direct solver (a sparse lu) instead of an iterative
// solver, use
//
//   -ksp_type preonly
//   -pc_type lu
//
// When PETSc is compiled with external direct solvers (UMFPACK,
// SuperLU, etc.), they can be selected like this:
//
//   -pc_factor_mat_solver_package umfpack

#include "GmshConfig.h"
#include "GmshMessage.h"
#include "linearSystem.h"
#include "sparsityPattern.h"
#include "fullMatrix.h"
#include <vector>
#if defined(HAVE_PETSC)

#ifndef SWIG
#include "petsc.h"
#include "petscksp.h"
#else
typedef struct _p_Mat* Mat;
typedef struct _p_Vec* Vec;
typedef struct _p_KSP* KSP;
#endif

//support old PETSc version, try to avoid using PETSC_VERSION in other places
#if PETSC_VERSION_RELEASE != 0 && (PETSC_VERSION_MAJOR < 3  || (PETSC_VERSION_MAJOR == 3 && PETSC_VERSION_MINOR < 2))
#define KSPDestroy(k) KSPDestroy(*(k))
#define MatDestroy(m) MatDestroy(*(m))
#define VecDestroy(v) VecDestroy(*(v))
#define PetscViewerDestroy(v) PetscViewerDestroy(*(v))
#define PetscBool PetscTruth
#define PetscOptionsGetBool PetscOptionsGetTruth
#endif


template <class scalar>
class linearSystemPETSc : public linearSystem<scalar> {
  protected:
  bool _isAllocated, _kspAllocated, _entriesPreAllocated;
  bool _matrixChangedSinceLastSolve;
  bool _valuesNotAssembled; //cannot use MatAssembled since MatAssembled return false for an empty matrix
  Mat _a;
  Vec _b, _x;
  KSP _ksp;
  int _localRowStart, _localRowEnd, _localSize, _globalSize;
  sparsityPattern _sparsity;
  void _kspCreate();
  void _assembleMatrixIfNeeded();
  #ifndef SWIG
  MPI_Comm _comm;
  #endif
  int _getBlockSizeFromParameters() const;
 public:
  ~linearSystemPETSc();
  void insertInSparsityPattern (int i, int j);
  bool isAllocated() const { return _isAllocated; }
  void preAllocateEntries();
  void allocate(int nbRows);
  void print();
  void clear();
  void getFromMatrix(int row, int col, scalar &val) const;
  void addToRightHandSide(int row, const scalar &val);
  void getFromRightHandSide(int row, scalar &val) const;
  double normInfRightHandSide() const;
  void addToMatrix(int row, int col, const scalar &val);
  void addToSolution(int row, const scalar &val);
  void getFromSolution(int row, scalar &val) const;
  void zeroMatrix();
  void zeroRightHandSide();
  void zeroSolution();
  void printMatlab(const char *filename) const;
  int systemSolve();
  Mat &getMatrix(){ return _a; }
  //std::vector<scalar> getData();
  //std::vector<int> getRowPointers();
  //std::vector<int> getColumnsIndices();
  #ifndef SWIG
  linearSystemPETSc(MPI_Comm com);
  MPI_Comm& getComm() {return _comm;}
  #endif
  linearSystemPETSc();
};
#else

template <class scalar>
class linearSystemPETSc : public linearSystem<scalar> {
 public :
  linearSystemPETSc()
  {
    Msg::Error("PETSc is not available in this version of Gmsh");
  }
  bool isAllocated() const { return false; }
  void allocate(int nbRows) {}
  void clear(){}
  void addToMatrix(int row, int col, const scalar &val) {}
  void getFromMatrix(int row, int col, scalar &val) const {}
  void addToRightHandSide(int row, const scalar &val) {}
  void addToSolution(int row, const scalar &val) {}
  void getFromRightHandSide(int row, scalar &val) const {}
  void getFromSolution(int row, scalar &val) const {}
  void zeroMatrix() {}
  void zeroRightHandSide() {}
  void zeroSolution() {}
  void printMatlab(const char *filename) const{};
  int systemSolve() { return 0; }
  double normInfRightHandSide() const{return 0;}
};
#endif
#endif
