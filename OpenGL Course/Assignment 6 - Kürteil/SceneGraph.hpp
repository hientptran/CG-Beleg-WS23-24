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
  void traversePicking(glm::mat4 modelView);

  // navigation in tree
  // (needed for node selection)
  void up();
  void down();
  void left();
  void right();
  void selectNode(Node targetNode);

  // increment / decrement rotation of selected node
  void rotate(float x, float y, float z);
  
  // reset all rotations
  void reset();

  void setProjectionMatrix(glm::mat4 projectionMatrix);
  void setViewMatrix(glm::mat4 viewMatrix);

  void addLightSource(LightSource lightSource);
  void addnodeMap(std::map<int, Node> nodeMap);
  void addRootNode(Node *node);
  
private:

  // traverse and draw the scenegraph from a given node
  void traverse(Node *node, glm::mat4 modelView);

  // traverse and draw the scenegraph from a given node
  void traversePicking(Node *node, glm::mat4 modelView);

  // reset all rotations below given node
  void reset(Node *node);

  // destroy all nodes
  void clear(Node *node);

  // root node
  Node* root;
  Node* rootNode;

  // currently selected node
  Node *selected;

  // select a node
  void selectNode(Node* node, Node targetNode);

  //glsl::Shader *shader;

  glm::mat4 projectionMatrix, viewMatrix;

  LightSource lightSource;
  std::map<int, Node> nodeMap;
};
