/* ----------------------------------------------------------------
   name:           Node.cpp
   purpose:        implementation for a scenegraph node
   version:	   STARTER CODE
   TODO:           render
   author:         katrin lang
                   htw berlin
   ------------------------------------------------------------- */

#include <iostream>

#include "Node.hpp"
#include "Log.hpp"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/string_cast.hpp"
//#include "glm/gtc/type_ptr.hpp"

#include <tuple>

// use this with care
// might cause name collisions
using namespace glm;
using namespace std;

// constructor
// x,y,z: position of node center of mass relative to parent node center
// length, height, and width of this body part
// jointx, jointy, jointz: x,y,z offset of  rotation axis from center of body part
// i.e. for rotation about a "shoulder" as opposed to the center of mass of a cube
// x,y,z: rotation (in degrees) of node relative to parent node rotation

Node::Node(float x, float y, float z, 
	   float length, float height, float width,
	   float jointx, float jointy, float jointz,
	   float rotx, float roty, float rotz){
  
  selected= false;
  
  position= vec3(x,y,z);
    
  dimension= vec3(length, height, width);
    
  joint= vec3(jointx, jointy, jointz);
    
  rotation= vec3(rotx, roty, rotz);

  rotationMatrix= mat4(1);
  
  parent= NULL;
  child= NULL;
  previous= NULL;
  next= NULL;
  
  reset();

  shader= NULL;
  mesh= NULL;
  material= NULL;
}

bool Node::operator<(const Node& other) const
{
    if (position.x < other.position.x) return true;
    if (position.x > other.position.x) return false;
    return false;
}

bool Node::operator==(const Node& other) const
{
    
    if (((position == other.position)   && (dimension == other.dimension)   && (joint == other.joint)) && (this->parent == other.parent)) return true;
    else return false;
}

void Node::setShader(glsl::Shader *shader){
  this->shader = shader;
}

void Node::setMesh(TriangleMesh *mesh){
  this->mesh= mesh;
}

void Node::setMaterial(Material *material){
  this->material= material;
}

void Node::setPickingShader(glsl::Shader *pickingShader) {
    this->pickingShader = pickingShader;
}

// destructor
Node::~Node(){
}

// sets the parent of this node
// and adds this node 
// to the list of the parent's children
void Node::setParent(Node* parent){
  
  this->parent= parent;
  if(parent->child==NULL){
    parent->child= this;
  }
  else{
    Node *sibling= parent->child;
    while(sibling->next != NULL) sibling= sibling->next;
    sibling->next= this;
    this->previous= sibling;
  }
}

// transform an individual node
// according to its position, 
// rotation, and rotation center
// and call draw()
void Node::render(mat4 projectionMatrix, mat4 viewMatrix, mat4 &modelMatrix, LightSource lightSource){

  // note the order of the operations:
  // the transformations are applied in "reverse" order
  // of glRotate/glTranslate calls to the node
  
  // TODO: translate to node center
    modelMatrix *= translate(position);
  
  // TODO: translate to joint position
    modelMatrix *= translate(joint);
  
  // TODO: apply this node's rotation
    modelMatrix *= rotationMatrix;

  // TODO: inverse translate to joint position
    modelMatrix *= glm::translate(-joint);

  // TODO: compute scaling matrix and pass its product with modelView to draw call
    mat4 scalingMatrix = glm::scale(dimension);
  
  draw(projectionMatrix, viewMatrix, modelMatrix*scalingMatrix, lightSource);
}

void Node::renderPicking(mat4 projectionMatrix, mat4 viewMatrix, mat4& modelMatrix, LightSource lightSource, vec4 color) {
    modelMatrix *= translate(position);
    modelMatrix *= translate(joint);
    modelMatrix *= rotationMatrix;
    modelMatrix *= glm::translate(-joint);
    mat4 scalingMatrix = glm::scale(dimension);
    drawPicking(projectionMatrix, viewMatrix, modelMatrix*scalingMatrix, lightSource, color);
}

// draw an individual node
void Node::draw(mat4 projectionMatrix, mat4 viewMatrix, mat4 modelMatrix, LightSource lightSource){
  
  glsl::Shader *shader= retrieveShader();
  TriangleMesh *mesh= retrieveMesh();
  Material *material= retrieveMaterial();

  shader->bind();

  shader->setUniform("transformation", projectionMatrix*viewMatrix*modelMatrix);
  shader->setUniform("lightPosition", inverse(modelMatrix)*lightSource.position);

  shader->setUniform("texturing", false);
  
  if(selected)
    shader->setUniform("color", vec4(1.0f, 0.3f, 0.3f, 1.0f));
  else
    shader->setUniform("color", material->diffuse);
  
  mesh->draw();
  
  shader->unbind();
}

void Node::drawPicking(mat4 projectionMatrix, mat4 viewMatrix, mat4 modelMatrix, LightSource lightSource, vec4 color) {

    //glsl::Shader* shader = retrieveShader();
    glsl::Shader* pickingShader = retrievePickingShader();
    TriangleMesh* mesh = retrieveMesh();

    //shader->bind();

    //shader->setUniform("transformation", projectionMatrix * viewMatrix * modelMatrix);
    //shader->setUniform("lightPosition", inverse(modelMatrix) * lightSource.position);

    //shader->setUniform("texturing", false);

    //shader->setUniform("color", color);

    /////////
    pickingShader->bind();
    pickingShader->setUniform("transformation", projectionMatrix * viewMatrix * modelMatrix);
    pickingShader->setUniform("PickingColor", color);
    /////////

    mesh->draw();

    //shader->unbind();
    pickingShader->unbind();
}

// increment / decrement rotation
void Node::rotate(float x, float y, float z){

  rotationMatrix*= glm::rotate(radians(x), vec3(1,0,0));
  rotationMatrix*= glm::rotate(radians(y), vec3(0,1,0));
  rotationMatrix*= glm::rotate(radians(z), vec3(0,0,1));
}

void Node::rotateWithMouse(float x, float y, float z) {

    rotationMatrix = glm::rotate(radians(x), vec3(1, 0, 0)) * rotationMatrix;   //todo: von rechts multiplizieren damit im Weltkoordinatensystem
    rotationMatrix = glm::rotate(radians(y), vec3(0, 1, 0)) * rotationMatrix;   //todo: x und y Achse verweselt
    rotationMatrix = glm::rotate(radians(z), vec3(0, 0, 1)) * rotationMatrix;
}

// retrieve inherited shader
glsl::Shader *Node::retrieveShader(){
  Node* node= this;
  while(!node->shader) node= node->getParent();
  return node->shader;
}

glsl::Shader* Node::retrievePickingShader() {
    Node* node = this;
    while (!node->pickingShader) node = node->getParent();
    return node->pickingShader;
}

// retrieve inherited mesh
TriangleMesh *Node::retrieveMesh(){
  Node* node= this;
  while(!node->mesh) node= node->getParent();
  return node->mesh;
}

// retrieve inherited material
Material *Node::retrieveMaterial(){
  Node* node= this;
  while(!node->material) node= node->getParent();
  return node->material;
}

// getter for next pointer
Node *Node::getNext(){
  return next;
}

// getter for previous pointer
Node *Node::getPrevious(){
  return previous;
}

// getter for parent pointer
Node *Node::getParent(){
  return parent;
}

// getter for child pointer
Node *Node::getChild(){
  return child;
}

// select node
void Node::select(){
  selected= true;
}

// deselect node
void Node::deselect(){
  selected= false;
}

// resets an individual node
// to the initial rotation 
// specified in constructor
void Node::reset(){

  rotationMatrix= mat4(1);
  rotationMatrix*= glm::rotate(radians(rotation.x), vec3(1.0f,0.0f,0.0f));
  rotationMatrix*= glm::rotate(radians(rotation.y), vec3(0.0f,1.0f,0.0f));
  rotationMatrix*= glm::rotate(radians(rotation.z), vec3(0.0f,0.0f,1.0f));
}
