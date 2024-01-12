#pragma once

/* ----------------------------------------------------------------
   name:           SceneGraph.hpp
   purpose:        scenegraph class declaration
   version:	   STARTER CODE
   TODO:           nothing (see SceneGraph.cpp)
   author:         katrin lang
                   htw berlin
   ------------------------------------------------------------- */

#include "Node.hpp"
#include "GLSL.hpp"
#include "LightSource.h"

class SceneGraph{

public:
    
  SceneGraph(Node *root);

  ~SceneGraph();

  // traverse and draw the scenegraph
  void traverse(glm::mat4 modelView);

  // navigation in tree
  // (needed for node selection)
  void up();
  void down();
  void left();
  void right();

  // increment / decrement rotation of selected node
  void rotate(float x, float y, float z);
  
  // reset all rotations
  void reset();

  void setProjectionMatrix(glm::mat4 projectionMatrix);
  void setViewMatrix(glm::mat4 viewMatrix);

  void addLightSource(LightSource lightSource);
  void addnodeMap(std::map<Node, int> nodeMap);
  
private:

  // traverse and draw the scenegraph from a given node
  void traverse(Node *node, glm::mat4 modelView);

  // reset all rotations below given node
  void reset(Node *node);

  // destroy all nodes
  void clear(Node *node);

  // root node
  Node *root; 

  // currently selected node
  Node *selected;

    glsl::Shader *shader;

  glm::mat4 projectionMatrix, viewMatrix;

  LightSource lightSource;
  std::map<Node, int> nodeMap;
};
