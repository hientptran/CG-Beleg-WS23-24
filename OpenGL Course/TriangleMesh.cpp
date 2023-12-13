/* -------------------------------------------------------------
   name:           TriangleMesh.cpp
   purpose:        cg1_ex1 triangle mesh for OpenGL rendering
   version:	   STARTER CODE
   author:         katrin lang
                   computer graphics
                   htw berlin
   ------------------------------------------------------------- */

#include "TriangleMesh.hpp"

// use this with care
// might cause name collisions
using namespace glm;

#include <fstream>
#include <iostream>
#include <sstream>

#include "GLError.hpp"

using namespace std;

// NVIDIA wants it like this
// see https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/attributes.php
const GLuint TriangleMesh::attribPosition= 0;
const GLuint TriangleMesh::attribNormal= 2;
const GLuint TriangleMesh::attribColor= 3;
const GLuint TriangleMesh::attribTexCoord= 8;

#ifdef __APPLE__
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#endif

TriangleMesh::TriangleMesh() : winding(CW),
			       vao(0),
			       ibo(0){
  vbo[0]=0;
  vbo[1]=0;
  vbo[2]=0;
}

TriangleMesh::~TriangleMesh(){
 
    clearBuffers();
}

void TriangleMesh::clearBuffers(void){

  // TODO (3.2): delete vao, vbo and ibo
    if (glIsVertexArray(vao)) glDeleteVertexArrays(1, &vao);

    if (glIsBuffer(vbo[0])) glDeleteBuffers(1, &vbo[0]);
    if (glIsBuffer(vbo[1])) glDeleteBuffers(1, &vbo[1]);
    if (glIsBuffer(vbo[2])) glDeleteBuffers(1, &vbo[2]);

    if (glIsBuffer(ibo)) glDeleteBuffers(1, &ibo);
}

void TriangleMesh::reload(void){

  load(name);
}

void TriangleMesh::load(const std::string& fileName, bool unitize){
  
  this->name= fileName;
  
  clearBuffers();

  positions.clear();
  normals.clear();
  texCoords.clear();
  faces.clear();
  
  if(fileName.size() > 3 && fileName.compare(fileName.size()-4, 4, ".off")==0)
    loadOff(fileName);
  else if(fileName.size() > 3 && fileName.compare(fileName.size()-4, 4, ".obj")==0)
    loadObj(fileName);
  else{
    cerr << "Unknown file format for triangle mesh. Must be .off or .obj." << endl;
    return;
  }

  center();
  if(unitize) this->unitize();
  if(normals.size()==0) computeNormals();
  if(texCoords.size()==0) computeSphereUVs();

  uploadBuffers();
}

void TriangleMesh::setWinding(PolygonWinding winding){
  this->winding= winding;
}

// center the mesh at its origin
void TriangleMesh::center(void){

  computeBoundingBox();

  vec3 center= (boundingBoxMin + boundingBoxMax) * vec3(0.5);
  
  for(unsigned int i= 0; i<positions.size(); i++){  
    positions[i]-= center;
  }
  boundingBoxMin-= center;
  boundingBoxMax-= center;
}

// scale to size 1
void TriangleMesh::unitize(void){

  computeBoundingSphere();

  for(unsigned int i= 0; i<positions.size(); ++i){
    positions[i]/= boundingSphereRadius;
  }
  boundingSphereRadius= 1;
  boundingBoxMin= vec3(-1);
  boundingBoxMax= vec3(1);
}

// compute bounding sphere
void TriangleMesh::computeBoundingSphere(void){

  boundingSphereRadius= 0;
  for(unsigned int i= 0; i<positions.size(); i++){
    vec3 v= positions[i];
    if(length(v) > boundingSphereRadius) boundingSphereRadius= length(v);
  }
}

// compute bounding box
void TriangleMesh::computeBoundingBox(void){

  boundingBoxMin= vec3(numeric_limits<float>::max());
  boundingBoxMax= vec3(numeric_limits<float>::min());
  for(unsigned int i= 0; i<positions.size(); i++){
    if(positions[i].x < boundingBoxMin.x) boundingBoxMin.x= positions[i].x;
    if(positions[i].x > boundingBoxMax.x) boundingBoxMax.x= positions[i].x;
    if(positions[i].y < boundingBoxMin.y) boundingBoxMin.y= positions[i].y;
    if(positions[i].y > boundingBoxMax.y) boundingBoxMax.y= positions[i].y;
    if(positions[i].z < boundingBoxMin.z) boundingBoxMin.z= positions[i].z;
    if(positions[i].z > boundingBoxMax.z) boundingBoxMax.z= positions[i].z;
  }
}

// TODO: load triangle mesh in OFF format
void TriangleMesh::loadOff(const string& fileName){
  
  // Open the file
  std::ifstream file(fileName.c_str());
  assert(file.is_open());
 
  std::string magic;

  // Read the first line, must be "OFF"
  file >> magic;
  assert(magic == "OFF");

  // number of vertices, faces and edges (unused)
  int V, F, E;
  file >> V >> F >> E;

  assert(file.good());

  // read all the vertices positions
  positions.resize(V);
  for (int i = 0; i < V; i++){

    float x,y,z;

    file >> x >> y >> z;

    assert(file.good());

    positions[i]= vec3(x, y, z);
  }

  // read number of vertices in the faces and the indices of the vertices
  faces.resize(F);
  for (int i = 0; i < F; i++){
    
    int n, a, b, c;
    
    file >> n >> a >> b >> c;
    
    assert(n == 3);
    assert(file.good());

    if(winding==CW)
      faces[i]= uvec3(a, b, c);
    else
      faces[i]= uvec3(a, c, b);
  }

  // close file
  file.close();
  cout << "Mesh: reading off file done. |V|=" << positions.size() << " |F|=" << faces.size() << endl;
}

// load an OBJ model
void TriangleMesh::loadObj(const string& fileName){
  
  /**
   * .obj file format
   *
   * '#'  -  comment
   * 'v'  -  vertex position (3 floats separated by whitespace)
   * 'vt' -  texture coordinates (2 floats separated by whitespace)
   * 'vn' -  vertex normal (3 floats separated by whitespace)
   * 'f'  -  faces, n times 3 v/vt/vn indices separated by /  and whitespace: v/t/n v/t/n ... v/t/n 
   */
  
  const string POSITION= "v";
  const string TEX_COORD= "vt";
  const string NORMAL= "vn";
  const string FACE= "f";
  
  ifstream file;
  file.open(fileName.c_str());
  
  vector<GLuint> pi;
  vector<GLuint> ti;
  vector<GLuint> ni;
  string l;
  
  while(getline(file, l)){  
    
    stringstream line(l);
    string type;
    line >> type;
    
    if(type == POSITION){
      
      float x, y, z;
      line >> x >> y >> z;
      positions.push_back(vec3(x,y,z));	
    }
    else if(type == TEX_COORD){
      
      float u, v;
      line >> u >> v;
      texCoordsRaw.push_back(vec2(u,v));
    }
    else if(type == NORMAL){
      
      float nx, ny, nz;
      line >> nx >> ny >> nz;
      normalsRaw.push_back(vec3(nx, ny, nz));
    }
    else if(type == FACE){
    
      pi.clear();
      ti.clear();
      ni.clear();
      GLuint positionIndex, texCoordIndex, normalIndex;
      char slash;
      
      while(!line.eof()){
  
	// v v v
	if(line.str().find('/') == string::npos && line.str().length() > 5){
	  line >> positionIndex;
	  pi.push_back(positionIndex-1);
	}
	// v//n v//n v//n
	else if(line.str().find("//") != string::npos){
	  line >> positionIndex >> slash >> slash >> normalIndex;
	  pi.push_back(positionIndex-1);
	  ni.push_back(normalIndex-1);
	}
	else{
	  // v/t v/t v/t
	  line >> positionIndex >> slash >> texCoordIndex;
	  pi.push_back(positionIndex-1);
	  ti.push_back(texCoordIndex-1);
	  // v/t/n v/t/n v/t/n
	  if(line.peek() == '/'){
	    line >> slash >> normalIndex;
	    ni.push_back(normalIndex-1);
	  }
	}
      } // line end
      triangulate(pi, ti, ni);
    } // end face
  }
  file.close();	    
  
  // bring to format opengl eats
  // this means possible duplication of normals 
  // and / or texture coordinates
  clean();
  
  cout << "loaded " << fileName << ": " 
       << positions.size() << " vertices, " << texCoordsRaw.size() << " texture coordinates, " << normalsRaw.size() << " normals, "
       << faces.size() << " faces" << endl;		
}

// triangulate a polygon
void TriangleMesh::triangulate(vector<GLuint> pi, vector<GLuint> ti, vector<GLuint> ni){
    
  for(unsigned int v= 2; v<pi.size(); v++){

    if(winding==CW)
      faces.push_back(uvec3(pi[0], pi[v-1], pi[v]));
    else
      faces.push_back(uvec3(pi[v], pi[v-1], pi[0]));
  }
  
  for(size_t v= 2; v<ti.size(); v++){
    
    if(winding==CW)
      texCoordIndices.push_back(uvec3(ti[0], ti[v-1], ti[v]));
    else
      texCoordIndices.push_back(uvec3(ti[v], ti[v-1], ti[0]));
  }
  
  for(unsigned int v= 2; v<ni.size(); v++){
    
    if(winding==CW)
      normalIndices.push_back(uvec3(ni[0], ni[v-1], ni[v]));
    else
      normalIndices.push_back(uvec3(ni[v], ni[v-1], ni[0]));
  }
  
  pi.clear();
  ti.clear();
  ni.clear();
}

// bring to format opengl eats
// this means possible duplication of normals 
// and / or texture coordinates
void TriangleMesh::clean(void){
  
  if(normalIndices.size()>0){
    normals.clear();
    normals.resize(positions.size());
    for(unsigned int i= 0; i<normalIndices.size(); i++){    
      normals[faces[i][0]]= normalsRaw[normalIndices[i][0]];
      normals[faces[i][1]]= normalsRaw[normalIndices[i][1]];
      normals[faces[i][2]]= normalsRaw[normalIndices[i][2]];
    }
    normalsRaw.clear();
    normalIndices.clear();
  }

  if(texCoordIndices.size()>0){
    texCoords.clear();
    texCoords.resize(positions.size());
    for(unsigned int i= 0; i<texCoordIndices.size(); i++){
      texCoords[faces[i][0]]= texCoordsRaw[texCoordIndices[i][0]];
      texCoords[faces[i][1]]= texCoordsRaw[texCoordIndices[i][1]];
      texCoords[faces[i][2]]= texCoordsRaw[texCoordIndices[i][2]];
    }
    texCoordsRaw.clear();
    texCoordIndices.clear();
  }
}

// parse a material library	
void TriangleMesh::parseMaterials(string filename){
	 
  const string NEW_MATERIAL= "newmtl";
  const string MATERIAL_AMBIENT= "Ka";
  const string MATERIAL_DIFFUSE= "Kd";
  const string MATERIAL_SPECULAR= "Ks";
  const string SPECULAR_EXPONENT= "Ns";

	    ifstream file;
	    file.open(filename.c_str());
	    
	    string l, current;

	    while(getline(file, l)){  

	      stringstream line(l);
	      string type;
	      line >> type;

	      if(type == NEW_MATERIAL){
		
		string name;
		line >> name;
		Material material;
		material.name= name;
		materials[name]= material;
		current= name;
	      }
	      else if(type == MATERIAL_AMBIENT){

		float r, g, b;
		line >> r >> g >> b;
		vec4 ambient= vec4(r,g,b,1.0f);
		materials[current].ambient= ambient;
	      }
	      else if(type == MATERIAL_DIFFUSE){

		float r, g, b;
		line >> r >> g >> b;
		vec4 diffuse= vec4(r,g,b,1.0f);
		materials[current].diffuse= diffuse;
	      }
	      else if(type == MATERIAL_SPECULAR){

		float r, g, b;
		line >> r >> g >> b;
		vec4 specular= vec4(r,g,b,1.0f);
		materials[current].specular= specular;
	      }
	      else if(type == SPECULAR_EXPONENT){

		float exponent;
		line >> exponent;
		materials[current].shininess= exponent;
	      }
	    } // eof
	    file.close();
}

/**
 * compute smooth per-vertex normals
 * by means of simple averaging
 **/
void TriangleMesh::computeNormals(void){
 
  normals.clear();
  normals.resize(positions.size());

  for(size_t f= 0; f<faces.size(); f++){

    // face vertices
    vec3 v0= positions[faces[f].x];
    vec3 v1= positions[faces[f].y];
    vec3 v2= positions[faces[f].z];

    // edges
    vec3 edge0= normalize(v0 - v1);
    vec3 edge1= normalize(v1 - v2);

    // face normal
    vec3 n= normalize(cross(edge0, edge1));

    // push out on vertices
    normals[faces[f][0]]+= n;
    normals[faces[f][1]]+= n;   
    normals[faces[f][2]]+= n;
  }

  //normalize
  for(size_t i= 0; i< normals.size(); i++){
    normals[i]= normalize(normals[i]);
  }
}

// Compute uv coordinates with a spherical mapping
// (vertices are projected on a sphere along the normal and classical sphere uv unwrap is used)
void TriangleMesh::computeSphereUVs(void){

  texCoords.resize(positions.size());
  
  // for each vertex
  for(unsigned int i = 0; i < positions.size(); i++){

    float x= positions[i].z;
    float y= positions[i].x;
    float z= -positions[i].y;

    float radius= glm::sqrt(x*x + y*y + z*z);
    float theta= acos(z/radius);
    float phi= atan2(y, x);
    
    texCoords[i]= vec2((pi<float>()+phi)/(2*pi<float>()), theta/pi<float>());
  }

  // texture coordinate correction

  vec3 u;

  for(unsigned int f = 0; f < faces.size(); ++f){

    uvec3 face= faces[f];

    u[0]= texCoords[face[0]].x;
    u[1]= texCoords[face[1]].x;
    u[2]= texCoords[face[2]].x;

    float threshold = 0.7f;

    if (fabs(u[0] - u[1]) > threshold || fabs(u[0] - u[2]) > threshold || fabs(u[1] - u[2]) > threshold){

      float middle = 0.5f;

      if (u[0] > middle && u[1] > middle && u[2] > middle)
	continue;

      for(unsigned int i= 0; i<3; i++){

	if (u[i] > middle){
	  positions.push_back(positions[face[i]]);
	  normals.push_back(normals[face[i]]);
	  texCoords.push_back(vec2(u[i] - 1, texCoords[face[i]].y));
	  faces[f][i] = positions.size() - 1;
	}
      }
    }
  }
}

// called whenever a new mesh is loaded
void TriangleMesh::uploadBuffers(void){
  
  glGenVertexArrays(1, &vao);
  assert(vao);
  
  // TODO (3.1): generate IDs for vertex buffer object 
  glGenBuffers(3, &vbo[0]);
  
  // TODO (3.1): generate IDs for index buffer object
  glGenBuffers(1, &ibo);
  
  // TODO (3.3): bind vertex array object for first time (begin record)
  // bind vertex array object for first time (begin record)
  glBindVertexArray(vao);

  // TODO (3.3): activate vertex position array
  // modifies the state of the currently bound vertex array object
  // activate vertex position array
  // modifies the state of the currently bound vertex array object
  glEnableVertexAttribArray(attribPosition);

  // TODO (3.3): bind buffer object
  // bind buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

  // TODO (3.4): send vertex positions to GPU
  // specify from where to read the position data and how many bytes to read
  glBufferData(GL_ARRAY_BUFFER,
      positions.size() * sizeof(vec3), // how many bytes?
      &positions[0], // location of position data
      GL_STATIC_DRAW); // no animation (hint)
  // how is the buffer data to be interpreted?

  glVertexAttribPointer(attribPosition, // vertex attribute
      3, // size of n-tuple (here: vec3)
      // making up a position
      GL_FLOAT, // data type of elements in n-tuple
      GL_FALSE, // normalize?
      0, // stride
      (GLvoid*)NULL); // take data from bound buffer
  
  // TODO (3.7): send vertex normals to GPU
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glEnableVertexAttribArray(attribNormal);
  glBufferData(GL_ARRAY_BUFFER,
      normals.size() * sizeof(vec3),
      &normals[0],
      GL_STATIC_DRAW);

  glVertexAttribPointer(attribNormal,
      3, // size of n-tuple (here: vec3)
      // making up a position
      GL_FLOAT, // data type of elements in n-tuple
      GL_TRUE, // normalize?
      0, // stride
      (GLvoid*)NULL); // take data from bound buffer

  // TODO (3.8): send texture coordinates to GPU
  glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
  glEnableVertexAttribArray(attribTexCoord);
  glBufferData(GL_ARRAY_BUFFER,
      texCoords.size() * sizeof(vec2),
      &texCoords[0],
      GL_STATIC_DRAW);

  glVertexAttribPointer(attribTexCoord,
      2, // size of n-tuple (here: vec3)
      // making up a position
      GL_FLOAT, // data type of elements in n-tuple
      GL_FALSE, // normalize?
      0, // stride
      (GLvoid*)NULL); // take data from bound buffer

  // TODO (3.5): send index buffer to GPU
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // for indices
  // specify buffer data for triangles
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      faces.size() * sizeof(uvec3), // how many bytes?
      &faces[0], // location of index data
      GL_STATIC_DRAW); // no animation (hint)|
  
  // TODO (3.5): unbind vertex array object and end record
  glBindVertexArray(0);
}

// called every frame
void TriangleMesh::draw(void) {
  glBindVertexArray(vao);
  
  // TODO (3.6): remove old code for vertex arrays
  /*
  glEnableVertexAttribArray(attribPosition);  // activate vertex coords array
  glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, &positions[0]);
  glEnableVertexAttribArray(attribNormal); // activate normal coords array
  glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_TRUE, 0, &normals[0]);
  glEnableVertexAttribArray(attribTexCoord); // activate texture coords array
  glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, 0, &texCoords[0]);
  */
  
  // TODO (3.6): make last argument a null pointer
  glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, nullptr); 

  // TODO (3.6): remove old code for vertex arrays
  /*
  glDisableVertexAttribArray(attribPosition);
  glDisableVertexAttribArray(attribNormal);
  glDisableVertexAttribArray(attribTexCoord);
  */
  glBindVertexArray(0);
}
