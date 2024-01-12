/* ---------------------------------------------------------------
   name:           Futurama.cpp
   purpose:        robot control 
   version:	   STARTER CODE
   author:         katrin lang
                   htw berlin
   ------------------------------------------------------------- */

#include "GLIncludes.hpp"

#include <iostream>
#include <iomanip> 
#include <sstream>

#include "Futurama.hpp"
#include "Context.hpp"
#include "Input.hpp"
#include "Robot.hpp"

#include "../LightSource.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.hpp"

#include "GLSL.hpp"

// use this with care
// might cause name collisions
using namespace glm;
using namespace std;

// field of view
GLfloat Futurama::fov= 55;
GLfloat Futurama::cameraZ= 15;

mat4 Futurama::projectionMatrix, Futurama::orthographicProjectionMatrix, Futurama::viewMatrix;

LightSource Futurama::lightSource={
    // position
  glm::vec4(0, 50, 300, 1),
  // ambient color
  glm::vec4(1.0, 1.0, 1.0, 1.0),
  // diffuse color
  glm::vec4(1.0, 1.0, 1.0, 1.0),
  // specular color
  glm::vec4(1.0, 1.0, 1.0, 1.0),
};

SceneGraph *Futurama::sceneGraph= NULL;
bool Futurama::bender= false;

// Create and compile our GLSL program from the shaders
glsl::Shader Futurama::pickingShader;

void ScreenPosToWorldRay(
	int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	int screenWidth, int screenHeight,  // Window size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
) {

	// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
	glm::vec4 lRayStart_NDC(
		((float)mouseX / (float)screenWidth - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
		((float)mouseY / (float)screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
		-1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
		1.0f
	);
	glm::vec4 lRayEnd_NDC(
		((float)mouseX / (float)screenWidth - 0.5f) * 2.0f,
		((float)mouseY / (float)screenHeight - 0.5f) * 2.0f,
		0.0,
		1.0f
	);


	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(ProjectionMatrix) goes from NDC to Camera Space.
	glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);

	// The View Matrix goes from World Space to Camera Space.
	// So inverse(ViewMatrix) goes from Camera Space to World Space.
	glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);

	glm::vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera /= lRayStart_camera.w;
	glm::vec4 lRayStart_world = InverseViewMatrix * lRayStart_camera; lRayStart_world /= lRayStart_world.w;
	glm::vec4 lRayEnd_camera = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera /= lRayEnd_camera.w;
	glm::vec4 lRayEnd_world = InverseViewMatrix * lRayEnd_camera;   lRayEnd_world /= lRayEnd_world.w;


	// Faster way (just one inverse)
	//glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
	//glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	//glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;


	glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
	lRayDir_world = glm::normalize(lRayDir_world);


	out_origin = glm::vec3(lRayStart_world);
	out_direction = glm::normalize(lRayDir_world);
}


bool TestRayOBBIntersection(
	glm::vec3 ray_origin,        // Ray origin, in world space
	glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
	glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
) {

	// Intersection method from Real-Time Rendering and Essential Mathematics for Games

	float tMin = 0.0f;
	float tMax = 100000.0f;

	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

	glm::vec3 delta = OBBposition_worldspace - ray_origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if (fabs(f) > 0.001f) { // Standard case

			float t1 = (e + aabb_min.x) / f; // Intersection with the "left" plane
			float t2 = (e + aabb_max.x) / f; // Intersection with the "right" plane
			// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection, 
			// so if it's not the case, invert t1 and t2
			if (t1 > t2) {
				float w = t1;t1 = t2;t2 = w; // swap t1 and t2
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if (t2 < tMax)
				tMax = t2;
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if (t1 > tMin)
				tMin = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin)
				return false;

		}
		else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Y axis
	// Exactly the same thing than above.
	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + aabb_min.y) / f;
			float t2 = (e + aabb_max.y) / f;

			if (t1 > t2) { float w = t1;t1 = t2;t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Z axis
	// Exactly the same thing than above.
	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + aabb_min.z) / f;
			float t2 = (e + aabb_max.z) / f;

			if (t1 > t2) { float w = t1;t1 = t2;t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;
	return true;

}

const OpenGLApplication::Config Futurama::config(glm::uvec2(2, 1),
						Config::Profile::COMPATIBILITY,
						(Window::DOUBLE | Window::DEPTH | Window::RGB | Window::MULTISAMPLE),
						8,
						glm::uvec2(50, 50),
						uvec2(512, 512),
						"Computer Graphics Assignment 2 - Futurama");

void Futurama::init(){
  
  // set background color to black
  //  glClearColor(0.718,0.808,0.857,1.0);
  glClearColor(0,0,0,1.0);
  glPolygonMode(GL_FRONT, GL_FILL);
  
  // enable depth test (z-buffer)
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE0);
  
  // enable antialiasing
  glEnable(GL_MULTISAMPLE);
}

// add a scenegraph
void Futurama::addSceneGraph(SceneGraph *sceneGraph){
  
  sceneGraph->addLightSource(lightSource);
  Futurama::sceneGraph= sceneGraph;
}

// adjust to new window size
void Futurama::reshape(){
  
  // viewport
  glViewport(0, 0, (GLsizei) Context::window->width(), (GLsizei) Context::window->height());
  
  computeViewMatrix();
  computeProjectionMatrix();
  orthographicProjectionMatrix= ortho(0.0f, (float) Context::window->width(), (float) Context::window->height(), 0.0f, -0.1f, 0.1f);
}

void Futurama::computeViewMatrix(void){

  // position the camera looking down the
  // negative z-axis at the center of the
  // world coordinate system. z-Position
  // of camera is stored in Futurama::cameraZ

  // Aufgabe 1: compute viewMatrix
    viewMatrix = lookAt(vec3(0.0, 0.0, cameraZ), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
  
  sceneGraph->setViewMatrix(viewMatrix);
}

void Futurama::computeProjectionMatrix(void){

  // reshaped window aspect ratio
  float aspect= (float) Context::window->width() / (float) Context::window->height();
  
  // compute near and far plane
  float nearPlane= cameraZ/10.0f; //near plane immer kleiner
  float farPlane= cameraZ*10.0f; //far plane immer größer
  
  // fH = tan( (fovY / 2) / 180 * pi ) * zNear;
  float fH = glm::tan( fov / 360 * glm::pi<float>()) * nearPlane;
  float fW = fH * aspect;

  // Aufgabe2: compute projection matrix via glm::perspective
  //projectionMatrix= glm::frustum( -fW, fW, -fH, fH, nearPlane, farPlane );
  projectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
  
  sceneGraph->setProjectionMatrix(projectionMatrix);
}

// this is where the drawing happens
void Futurama::display(void){
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw the scenegraph
  sceneGraph->traverse(mat4(1));

  drawCameraParameters();
  
  // display back buffer
  Context::window->swapBuffers();
}

void Futurama::drawCameraParameters(void){
  
  glColor3f(1,1,1);
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(&orthographicProjectionMatrix[0][0]);

  Input::setFont("helvetica", 12);
  ostringstream s;
  s << "fov (f/F): " << fov << endl;
  Input::drawString(15, Context::window->width()-35, s.str());
  s.str("");
  s << "camera z (c/C): " << fixed << setprecision(2) << cameraZ << endl;
  Input::drawString(15, Context::window->width()-15, s.str());

  glPopMatrix();
}

// keyboard control
void Futurama::keyPressed(){

  // rotation units
  float step= 2.0;
  
  // rotate selected node around 
  // x,y and z axes with keypresses
  switch(Input::keyboard.key){
    
  case 'q':
  case 'Q': exit(0);
  case 'x':
    sceneGraph->rotate(step, 0, 0);
    Context::window->redisplay();
    break;
  case 'X': 
    sceneGraph->rotate(-step, 0, 0);
    Context::window->redisplay();
    break;
  case 'y':
    sceneGraph->rotate(0, step, 0);
    Context::window->redisplay();
    break;
  case 'Y' :
    sceneGraph->rotate(0, -step, 0);
    Context::window->redisplay();
    break;  
  case 'z':
    sceneGraph->rotate(0, 0, step);
    Context::window->redisplay();
    break;
  case 'Z':
    sceneGraph->rotate(0, 0, -step);
    Context::window->redisplay();
    break;
  case 'f':
    if(fov>0) fov--;
    computeProjectionMatrix();
    Context::window->redisplay();
    break;
  case 'F': 
    if(fov<180) fov++;
    computeProjectionMatrix();
    Context::window->redisplay();
    break;
  case 'c':
    if(cameraZ>0) cameraZ-=0.1;
    computeViewMatrix();
    computeProjectionMatrix();
    Context::window->redisplay();
    break;
  case 'C':
    cameraZ+=0.1;
    computeViewMatrix();
    computeProjectionMatrix();
    Context::window->redisplay();
    break;
  case 'r': 
    sceneGraph->reset();
    Context::window->redisplay();
    break;
  case Keyboard::Key::SPECIAL:
    handleSpecialKeys();
    break;  
  default:
    break;
  }
}

// keyboard callback for special keys
// (arrow keys for node selection)
void Futurama::handleSpecialKeys(){

  // rotate selected node around 
  // x,y and z axes with keypresses
  switch(Input::keyboard.code) {

  case Keyboard::Code::UP:
    sceneGraph->up();
    Context::window->redisplay();
    break;
  case Keyboard::Code::DOWN:
    sceneGraph->down();
    Context::window->redisplay();
    break;
  case Keyboard::Code::LEFT:
    sceneGraph->left();
    Context::window->redisplay();
    break;
  case Keyboard::Code::RIGHT:
    sceneGraph->right();
    Context::window->redisplay();
    break;
  default:
    break;
  }
}

void Futurama::mousePressed()
{
	// Get mouse position
	ivec2 xpos, ypos = Input::mouse.position;
	std::cout << "mouse pos: " << xpos.x << ", " << xpos.y << ", " << ypos.x << ypos.y << std::endl;

	//Input::mouse.position = xpos, ypos;
	//glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	//glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Compute the MVP matrix from keyboard and mouse input
	//computeMatricesFromInputs();

	// reshaped window aspect ratio
	float aspect = (float)Context::window->width() / (float)Context::window->height();

	// compute near and far plane
	float nearPlane = cameraZ / 10.0f; //near plane immer kleiner
	float farPlane = cameraZ * 10.0f; //far plane immer größer

	// fH = tan( (fovY / 2) / 180 * pi ) * zNear;
	float fH = glm::tan(fov / 360 * glm::pi<float>()) * nearPlane;
	float fW = fH * aspect;

	projectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
	glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
	//glm::mat4 ProjectionMatrix = getProjectionMatrix();

	viewMatrix = lookAt(vec3(0.0, 0.0, cameraZ), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	glm::mat4 ViewMatrix = lookAt(vec3(0.0, 0.0, cameraZ), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	//glm::mat4 ViewMatrix = getViewMatrix();

	glm::vec3 ray_origin;
	glm::vec3 ray_direction;
	ScreenPosToWorldRay(
		512 / 2, 512 / 2,
		512, 512,
		ViewMatrix,
		ProjectionMatrix,
		ray_origin,
		ray_direction
	);

	std::vector<glm::vec3> positions(8);
	positions[0] = glm::vec3(0, 2.5, 0);
	positions[1] = glm::vec3(0, 1, 0);
	positions[2] = glm::vec3(-1.75, 0, 0);
	positions[3] = glm::vec3(1.75, 0, 0);
	positions[4] = glm::vec3(-0.5, -3.5, 0);
	positions[5] = glm::vec3(0.5, -3.5, 0);
	positions[6] = glm::vec3(0, -1.75, 0);
	positions[7] = glm::vec3(0, -1.75, 0);

	std::vector<glm::vec3> orientations(8);
	orientations[0] = glm::vec3(0, 0, 0);
	orientations[1] = glm::vec3(0, 0, 0);
	orientations[2] = glm::vec3(0, 0, 0);
	orientations[3] = glm::vec3(0, 0, 0);
	orientations[4] = glm::vec3(0, 0, 0);
	orientations[5] = glm::vec3(0, 0, 0);
	orientations[6] = glm::vec3(0, 0, 0);

	for (int i = 0; i < 8; i++) {

		float intersection_distance; // Output of TestRayOBBIntersection()
		glm::vec3 aabb_min(-1.0f, -1.0f, -1.0f);
		glm::vec3 aabb_max(1.0f, 1.0f, 1.0f);

		// The ModelMatrix transforms :
		// - the mesh to its desired position and orientation
		// - but also the AABB (defined with aabb_min and aabb_max) into an OBB
		glm::mat4 RotationMatrix = glm::mat4(0);
		glm::mat4 TranslationMatrix = translate(mat4(), positions[i]);
		glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;
		std::string message;
		if (TestRayOBBIntersection(
			ray_origin,
			ray_direction,
			aabb_min,
			aabb_max,
			ModelMatrix,
			intersection_distance)
			) {

			std::cout << "ray_origin: " << ray_origin.x << ", " << ray_origin.y << ", " << ray_origin.z << std::endl;
			std::cout << "ray_direction: " << ray_direction.x << ", " << ray_direction.y << ", " << ray_direction.z << std::endl;
			std::cout << "aabb_min: " << aabb_min.x << ", " << aabb_min.y << ", " << aabb_min.z << std::endl;
			std::cout << "aabb_max: " << aabb_max.x << ", " << aabb_max.y << ", " << aabb_max.z << std::endl;
			std::cout << "intersection_distance: " << intersection_distance << std::endl;
			std::ostringstream oss;
			oss << "mesh " << i;
			message = oss.str();
			break;
		}
	}
}

vector< pair < int, string > > Futurama::menuEntries{{Menu::QUIT, "quit"},
						     {Menu::RESET, "reset"},
						     {Menu::BENDER, "toggle [b]ender"}};
						      
// mouse menu control
void Futurama::menu(int id){
  
  switch (id) {
  case Menu::QUIT: 
    exit(0);
 
  case Menu::RESET:
    sceneGraph->reset();
    Context::window->redisplay();
    break;
    
  default:
    break;
  }
}

int main(int argc, char** argv){

  // initialize OpenGL context
  OpenGLContext<Futurama>::init(&argc, argv);

  // some diagnostic output
  std::cout << "GPU: " << glGetString(GL_RENDERER) << ", OpenGL version: " << glGetString(GL_VERSION) << ", GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  
  //  build the robot hierarchy (see robot.cpp)
  Node *root= Robot::build();
  
  //make scenegraph
  SceneGraph *sceneGraph= new SceneGraph(root);

  Futurama::addSceneGraph(sceneGraph);
 
  //start event loop
  InputManager<Futurama>::startEventLoop();
  
  return 0;
}

