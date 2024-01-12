/* ----------------------------------------------------------------
   name:           Scenegraph.cpp
   purpose:        implementation of the scenegraph functions
   version:	   STARTER CODE
   TODO:           traverse, reset
   author:         katrin lang
                   htw berlin
   ------------------------------------------------------------- */

#include "GLIncludes.hpp"

#include <iostream>

#include "glm/glm.hpp"

#include "SceneGraph.hpp"

// use this with care
// might cause name collisions
using namespace std;
using namespace glm;

SceneGraph::SceneGraph(Node *root) : root(root)
				   , selected(root)
				   , projectionMatrix(1)
				   , viewMatrix(1){

  selected->select();
}

SceneGraph::~SceneGraph(){

  clear(root);
}

// traverse and draw the scenegraph
// nothing to do here
// (see helper function)
void SceneGraph::traverse(mat4 modelView){
 
  traverse(root, modelView);
}

// reset all rotations in the scenegraph
// nothing to do here
// (see helper function)
void SceneGraph::reset(){
  reset(root);
}

// navigation in tree
// (needed for node selection)
void SceneGraph::up(){
  if(selected->getParent() == NULL) return;
  selected->deselect();
  selected= selected->getParent();
  selected->select();
}
void SceneGraph::down(){
  if(selected->getChild() == NULL) return;
  selected->deselect();
  selected= selected->getChild();
  selected->select();
}
void SceneGraph::left(){
  if(selected->getPrevious() == NULL) return;
  selected->deselect();
  selected= selected->getPrevious();
  selected->select();
}
void SceneGraph::right(){ 
  if(selected->getNext() == NULL) return;
  selected->deselect();
  selected= selected->getNext();
  selected->select();
}

// increment / decrement rotation of selected node
void SceneGraph::rotate(float x, float y, float z){
  selected->rotate(x, y, z);
}

// traverse and draw the scenegraph from a given node
void SceneGraph::traverse(Node *node, mat4 modelMatrix){

	// Aufgabe 6: recursion anchor
	if (node == NULL) return;	// 1) Abbruchbedingung

	//int nodeID = find(nodeVector.begin(), nodeVector.end(), node);
  
	// Aufgabe 6: traverse possible siblings		// 3) Objekt Transformieren
	traverse(node->getNext(), modelMatrix);	// 2) Unveränderte Einheitsmatrix an Siblings verteilen
  
	// render node and concatenate its transformation onto modelMatrix
    node->render(projectionMatrix, viewMatrix, modelMatrix, lightSource);

    // Aufgabe 6: traverse possible children		// 4) Transformierte Matrix an Children verteilen
    traverse(node->getChild(), modelMatrix); //aber eigentlich neue Matrix übergeben
}

void SceneGraph::clear(Node *node){

  if(node == NULL) return;
  delete node;
  clear(node->getChild());
  clear(node->getNext());
}

// reset the subtree corresponding 
// to a given node
void SceneGraph::reset(Node* node){

  //reset sceneGraph
	if (node == NULL) return;
	reset(node->getNext());
	node->rotationMatrix = mat4(1);
	reset(node->getChild());

}

void SceneGraph::setProjectionMatrix(mat4 projectionMatrix){
  this->projectionMatrix= projectionMatrix;
}

void SceneGraph::setViewMatrix(mat4 viewMatrix){
  this->viewMatrix= viewMatrix;
}

void SceneGraph::addLightSource(LightSource lightSource){
  this->lightSource= lightSource;
}

void SceneGraph::addNodeVector(std::map<Node, int> nodeVector) {
	this->nodeVector = nodeVector;
}
