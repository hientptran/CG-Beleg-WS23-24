#pragma once

/* ----------------------------------------------------------------
   name:           Application.hpp
   purpose:        base class for OpenGL applications
   version:	   LIBRARY CODE
   TODO:           nothing
   author:         katrin lang
		   htw berlin
   ------------------------------------------------------------- */

#include "glm/glm.hpp"
#include <vector>
#include <string>
// for pair
#include <utility>

#include "Window.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

class OpenGLApplication{
  
public:

  struct Config{
    
    glm::uvec2 version;
    enum Profile{CORE, COMPATIBILITY} profile;
    unsigned int flags;
    unsigned int samples;
    glm::uvec2 position;
    glm::uvec2 size;
    std::string title;
    
    Config(glm::uvec2 version, Profile profile, unsigned int flags, unsigned int samples, glm::uvec2 position, glm::uvec2 size, std::string title);
  };
  
  static const Config getConfig(void);

  static const Config config;
  
  // initialization
  static void init(void);
  
  // called when redisplay was requested
  static void display(void);
 
  // called after window rehape
  static void reshape(void);

  // called when mouse pressed
  static void mousePressed(void);

  // called when mouse dragged
  static void mouseMoved(void);
  
  // called when mouse dragged
  static void mouseDragged(void);

  // called when mouse released
  static void mouseReleased(void);

  // called when key pressed
  static void keyPressed(void);

  // called when key released
  static void keyReleased(void);

  // called when menu has input
  static void menu(int);

  // called when idle
  static void idle(void);
  
  static std::vector<std::pair< unsigned int, std::string> > menuEntries;
};
