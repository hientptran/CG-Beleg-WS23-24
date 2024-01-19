/* ----------------------------------------------------------------
   name:           Robot.cpp
   purpose:        implementation of the scenegraph construction
   version:	   STARTER CODE
   TODO:           build
   author:         katrin lang
                   htw berlin
   ------------------------------------------------------------- */

// Include GLM
#include <glm/glm.hpp>

#include "Robot.hpp"
#include "../Material.h"

static TriangleMesh cube;
  
  // material parameters
static Material metal= {
  "metal",
  // ambient color
  glm::vec4(0.5, 0.5, 0.5, 1.0),
  // diffuse color
  glm::vec4(0.556, 0.659, 0.664, 1.0),
  // specular color
  glm::vec4(0.5, 0.5, 0.5, 1.0),
    // shininess
  3.0f
};

static glsl::Shader toonShader;
static glsl::Shader pickingShader;

//      CURRENTLY ONLY TORSO. INSERT OTHER BODY PARTS HERE.
//      SEE EXAMPLE BINARY Futurama_ref_windows.exe / Futurama_ref_macos / Futurama_ref_linux
//      EXPERIMENTATION IS ENCOURAGED. THE FINAL MODEL/SCENEGRAPH
//      NEED NOT BE IDENTICAL TO THE PROPOSED SOLUTION.

  // torso attributes and initial position.
  // for parameters of constructor and their meanings,
  // see file Node.hpp
static Node torso(0, 1, 0,
		  2.5, 3.5, 1.5,
		  0, -1, 0,
		  0, 0, 0);

// TODO: implement more body/robot/whatever parts here

// ML schnipp

static Node head(0, 2.5, 0,
		 1.5, 1, 0.75,
		 0, -0.5, 0,
		 0, 0, 0);

static Node antenna(0, 1, 0,
		    0.5, 0.4, 0.5,
		    0, -0.2, 0,
		    0, 0, 0);

static Node leftArm(-1.75, 0, 0,
		    0.4, 2.5, 0.4,
		    0.2, 1.5, 0,
		    0, 0, 0);

static Node rightArm(1.75, 0, 0,
		     0.4, 2.5, 0.4,
		     -0.2, 1.5, 0,
		     0, 0, 0);

static Node leftLeg(-0.5, -3.5, 0,
		    0.6, 3, 0.6,
		    0, 1.5, 0,
		    0, 0, 0);

static Node rightLeg(0.5, -3.5, 0,
		     0.6, 3, 0.6,
		     0, 1.5, 0,
		     0, 0, 0);

static Node leftFoot(0, -1.75, 0,
		     0.9, 0.25, 1.5,
		     0, 0, 0,
		     0, 0, 0);

static Node rightFoot(0, -1.75, 0,
		      0.9, 0.25, 1.5,
		      0, 0, 0,
		      0, 0, 0);

// ML schnapp

//static std::map<int, Node> nodeMap;
//static std::vector<Node> nodeMap{torso, head, antenna, leftArm, rightArm, leftLeg, rightLeg, leftFoot, rightFoot};
std::map<int, Node> Robot::nodeMap;

// build the robot scenegraph
Node *Robot::build(){
  
  // mesh
  cube.load("meshes/cube.obj", false);

  // shader

  const std::string version= "#version 130\n";
  /*
  diffuseShader.addVertexShader(version);
  diffuseShader.loadVertexShader("shaders/diffuse.vert");
  diffuseShader.compileVertexShader();
  diffuseShader.addFragmentShader(version);
  diffuseShader.loadFragmentShader("shaders/diffuse.frag");
  diffuseShader.compileFragmentShader();
  diffuseShader.bindVertexAttrib("position", TriangleMesh::attribPosition);
  diffuseShader.bindVertexAttrib("normal", TriangleMesh::attribNormal);
  diffuseShader.link();
  */

  toonShader.addVertexShader(version);
  toonShader.loadVertexShader("shaders/toon.vert");
  toonShader.compileVertexShader();
  toonShader.addFragmentShader(version);
  toonShader.loadFragmentShader("shaders/toon.frag");
  toonShader.compileFragmentShader();
  toonShader.bindVertexAttrib("position", TriangleMesh::attribPosition);
  toonShader.bindVertexAttrib("normal", TriangleMesh::attribNormal);
  toonShader.bindVertexAttrib("texCoord", TriangleMesh::attribTexCoord);
  toonShader.link();

  pickingShader.addVertexShader(version);
  pickingShader.loadVertexShader("shaders/picking.vert");
  pickingShader.compileVertexShader();
  pickingShader.addFragmentShader(version);
  pickingShader.loadFragmentShader("shaders/picking.frag");
  pickingShader.compileFragmentShader();
  pickingShader.bindVertexAttrib("position", TriangleMesh::attribPosition);
  pickingShader.link();
  
  // You dont't have to repeat this for every node.
  // These properties are inherited
  torso.setMesh(&cube);
  torso.setMaterial(&metal);
  torso.setPickingShader(&pickingShader);
  torso.setShader(&toonShader);
  
  // TODO: finally, attach the robot/model parts  

  // Schwerpunkt (Position des Objekts)
  // Größe des Objekts
  // Joint Position
  // initiale Rotation mit 0, 0, 0

  // ML schnipp
  head.setParent(&torso);
  antenna.setParent(&head);
  leftArm.setParent(&torso);
  rightArm.setParent(&torso);
  leftLeg.setParent(&torso);
  rightLeg.setParent(&torso);
  leftFoot.setParent(&leftLeg);
  rightFoot.setParent(&rightLeg);
  // ML schnapp

	nodeMap.emplace(0,torso);
	nodeMap.emplace(1,head);
	nodeMap.emplace(2,antenna);
	nodeMap.emplace(3,leftArm);
	nodeMap.emplace(4,rightArm);
	nodeMap.emplace(5,leftLeg);
	nodeMap.emplace(6,rightLeg);
	nodeMap.emplace(7,leftFoot);
	nodeMap.emplace(8,rightFoot);

  // return root node
  return &torso;
}
