#pragma once

/* ----------------------------------------------------------------
   name:           Control.hpp
   purpose:        robot control
   version:	   SKELETON CODE
   TODO:           nothing (see Control.cpp)
   author:         katrin lang
   computer graphics
   tu berlin
   ------------------------------------------------------------- */

#include <vector>
// for pair
#include <utility>

#include "glm/glm.hpp"

#include "Context.hpp"
#include "Input.hpp"
#include "Application.hpp"
#include "SceneGraph.hpp"

class Futurama : public OpenGLApplication{

  typedef OpenGLContext<Futurama> Context;
  typedef InputManager<Futurama> Input;
  
public:
  
  // initialization
  static void init();
  
  // add a scenegraph
  static void addSceneGraph(SceneGraph *sceneGraph, std::map<int, Node> nodeMap);

  // display scene
  static void display(void);
 
  // called after window rehape
  static void reshape();
 
  // called when menu item was selected          
  static void menu(int id);
  
  // keyboard callback
  static void keyPressed();

  // mouse click callback
  static void mousePressed();

  // keyboard callback for special keys 
  static void handleSpecialKeys();
    
  // menu entries
  static std::vector< std::pair< int, std::string > >  menuEntries;

  static const Config config;

  static glsl::Shader pickingShader;
  
private:

  static void computeViewMatrix(void);
  static void computeProjectionMatrix(void);

  static void drawCameraParameters(void);
  
  // the scenegraphs
  static SceneGraph *sceneGraph;

  static bool bender;
  
  // field of view (in degrees)                                               
  static GLfloat fov;

  static float pan;
  
  // camera position                                                           
  static float cameraZ;

  // near and far plane                                                        
  static float nearPlane, farPlane;

  static glm::mat4 projectionMatrix;
  static glm::mat4 orthographicProjectionMatrix;
  static glm::mat4 viewMatrix;

  static LightSource lightSource;
  static std::map<int, Node> nodeMap;
  
  struct Menu{
    enum Item{QUIT,
	      RESET,
	      BENDER,
	      TEXTURING,
	      
    };
  };
};
