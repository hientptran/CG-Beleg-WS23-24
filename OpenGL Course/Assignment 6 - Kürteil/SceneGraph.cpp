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
				   , viewMatrix(1) {

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

//std::map<int, Node> SceneGraph::nodeMap;
void SceneGraph::traversePicking(mat4 modelView) {

	traversePicking(root, modelView);
}

void SceneGraph::selectNode(Node targetNode) {

	selectNode(root, targetNode);
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

void SceneGraph::selectNode(Node *node, Node targetNode) {
	if (node == NULL) return;
	if (targetNode == *node) {
		selected->deselect();
		selected = node;
		selected->select();
		return;
	}
	selectNode(node->getNext(),targetNode);
	selectNode(node->getChild(), targetNode);
}

// increment / decrement rotation of selected node
void SceneGraph::rotate(float x, float y, float z){
  selected->rotate(x, y, z);
}

void SceneGraph::rotateWithMouse(float x, float y, float z) {
	selected->rotateWithMouse(x, y, z);
}

// traverse and draw the scenegraph from a given node
void SceneGraph::traverse(Node *node, mat4 modelMatrix){

	// Aufgabe 6: recursion anchor
	if (node == NULL) return;	// 1) Abbruchbedingung
  
	// Aufgabe 6: traverse possible siblings		// 3) Objekt Transformieren
	traverse(node->getNext(), modelMatrix);	// 2) Unveränderte Einheitsmatrix an Siblings verteilen
  
	// render node and concatenate its transformation onto modelMatrix
    node->render(projectionMatrix, viewMatrix, modelMatrix, lightSource);

    // Aufgabe 6: traverse possible children		// 4) Transformierte Matrix an Children verteilen
    traverse(node->getChild(), modelMatrix); //aber eigentlich neue Matrix übergeben
}

// Draw alternative colors for picking
void SceneGraph::traversePicking(Node *node, glm::mat4 modelMatrix) {
	if (node == NULL) return;	
	//node = *node.getNext();
	traversePicking(node->getNext(), modelMatrix);
	vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	int r = 0;
	int g = 0;
	int b = 0;
	for (int i = 1; i <= nodeMap.size(); i++) {
		color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		if (nodeMap.at(i) == *node) {
			// Convert "i", the integer mesh ID, into an RGB color
			r = (i & 0x000000FF) >> 0;
			g = (i & 0x0000FF00) >> 8;
			b = (i & 0x00FF0000) >> 16;
			//cout << r / 255.0f << ", " << g / 255.0f << ", " << b / 255.0f << endl;
			color = vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
			node->renderPicking(projectionMatrix, viewMatrix, modelMatrix, lightSource, color);
			break;
		} 
	}
	traversePicking(node->getChild(), modelMatrix);
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

void SceneGraph::addnodeMap(std::map<int, Node> nodeMap) {
	this->nodeMap = nodeMap;
}

void SceneGraph::addRootNode(Node* node) {
	this->rootNode = node;
}
