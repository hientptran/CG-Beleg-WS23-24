#include "Application.hpp"

/* ----------------------------------------------------------------
   name:           Application.cpp
   purpose:        base class for OpenGL applications
   version:	   LIBRARY CODE
   TODO:           nothing
   author:         katrin lang
		   htw berlin
   ------------------------------------------------------------- */

#include "GLIncludes.hpp"

OpenGLApplication::Config::Config(glm::uvec2 version, enum Config::Profile profile, unsigned int flags, unsigned int samples, glm::uvec2 position, glm::uvec2 size, std::string title)
  : version(version), profile(profile), flags(flags), samples(samples), position(position), size(size), title(title){}


const OpenGLApplication::Config OpenGLApplication::config(glm::uvec2(2, 1),
							  OpenGLApplication::Config::Profile::COMPATIBILITY,
							  (Window::DOUBLE | Window::DEPTH | Window::RGB | Window::MULTISAMPLE),
							  8,	
							  glm::uvec2(50,50),
							  glm::uvec2(600, 600),
							  "Hello, World");

std::vector< std::pair<unsigned int, std::string> > OpenGLApplication::menuEntries;

void OpenGLApplication::init(void){
  
}

// all callbacks that are not overriden deregister themselves

void OpenGLApplication::display(void){
  glutDisplayFunc(NULL);
}

void OpenGLApplication::reshape(void){
  glutReshapeFunc(NULL);
}

void OpenGLApplication::mousePressed(void){
  glutMouseFunc(NULL);
}

void OpenGLApplication::mouseMoved(void){
  glutPassiveMotionFunc(NULL);
}

void OpenGLApplication::mouseDragged(void){
   glutMotionFunc(NULL);
}

void OpenGLApplication::mouseReleased(void){

}

void OpenGLApplication::keyPressed(void){
  glutKeyboardFunc(NULL);
  glutSpecialFunc(NULL);
}

void OpenGLApplication::keyReleased(void){
  glutKeyboardUpFunc(NULL);
  glutSpecialUpFunc(NULL);
}

void OpenGLApplication::idle(void){
 glutIdleFunc(NULL);
}

void OpenGLApplication::menu(int){}
