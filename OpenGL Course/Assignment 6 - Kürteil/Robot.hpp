#pragma once

/* ----------------------------------------------------------------
   name:           Robot.h
   purpose:        the scenegraph/robot hierarchy/construction
   version:	   STARTER CODE
   TODO:           nothing (see Robot.cpp)
   author:         katrin lang
                   htw berlin
   ------------------------------------------------------------- */

#include "Node.hpp"
#include "TriangleMesh.hpp"
#include "GLSL.hpp"
#include "vector"

class Robot{

public:
  
  // construct robot hierarchy
  static Node *build();
  static std::map<Node, int> nodeMap;
};
