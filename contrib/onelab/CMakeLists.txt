# Gmsh - Copyright (C) 1997-2013 C. Geuzaine, J.-F. Remacle
#
# See the LICENSE.txt file for license information. Please report all
# bugs and problems to the public mailing list <gmsh@geuz.org>.

set(SRC
  OnelabClients.cpp 
  OnelabMessage.cpp
  OnelabParser.cpp
  metamodel.cpp
)

file(GLOB_RECURSE HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hxx)
append_gmsh_src(contrib/onelab "${SRC};${HDR}")
