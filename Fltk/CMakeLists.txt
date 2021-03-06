# Gmsh - Copyright (C) 1997-2013 C. Geuzaine, J.-F. Remacle
#
# See the LICENSE.txt file for license information. Please report all
# bugs and problems to the public mailing list <gmsh@geuz.org>.

set(SRC
  FlGui.cpp
    graphicWindow.cpp
      openglWindow.cpp
    optionWindow.cpp
      colorbarWindow.cpp
    drawContextFltkCairo.cpp
    drawContextFltkStringTexture.cpp
    gamepadWindow.cpp
    fieldWindow.cpp
    pluginWindow.cpp
    statisticsWindow.cpp
    visibilityWindow.cpp
    highOrderToolsWindow.cpp
    clippingWindow.cpp
    manipWindow.cpp
    contextWindow.cpp
    helpWindow.cpp
    fileDialogs.cpp
    extraDialogs.cpp
    projectionEditor.cpp
    classificationEditor.cpp
    partitionDialog.cpp
    onelabGroup.cpp
    inputValue.cpp inputRegion.cpp
    viewButton.cpp solverButton.cpp
    Navigator.cpp
)

file(GLOB HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h) 
append_gmsh_src(Fltk "${SRC};${HDR}")
