#pragma once

#include <string>

#define GLSL(code) (#code)

#ifndef EXTRA

const std::string version= "#version 120\n";

const std::string geometry_shader4= "#extension GL_EXT_geometry_shader4 : enable\n";

const std::string blinnPhongReflectionShader=

  GLSL(

       // global variables
       uniform LightSource lightSource;
       uniform Material material;

       // PLEASE NOTE: light source position, vertex position and vertex normal are in VIEW SPACE
       vec4 blinnPhongReflection(vec4 position, vec3 normal){

	    // TODO (4.1): ambient lighting: Ca = La Ma
            vec4 Ca = lightSource.ambient * material.ambient;

        // TODO (4.2): compute the light direction
            vec3 lightDirection = normalize(lightSource.position.xyz - position.xyz);
            //return vec4(lightDirection, 1);
       
	    // TODO (4.3): compute n dot l
            float nDotL = dot(normal, lightDirection);
            //return vec4(vec3(nDotL), 1);

	    // TODO (4.4): compute diffuse component
        // diffuse lighting: Cd= (n dot l) Ld Md
            vec4 Cd = max(nDotL,0) * lightSource.diffuse * material.diffuse;
            //return Cd;
     
	    // TODO (4.5): compute view direction
	    // hint: we are in eye coordinates, so eye position is (0,0,0)
            vec3 viewDirection = normalize(vec3(0, 0, 0) - position.xyz);
            //return vec4(viewDirection, 1);
	 
	    // TODO (4.6): compute half vector
            vec3 halfVector = normalize(lightDirection + viewDirection);
            //return vec4(halfVector, 1);
	 
	    // TODO (4.7): compute n dot h
            float nDotH = dot(normal, halfVector);
            //return vec4(vec3(nDotH), 1);

        // TODO (4.8): compute specular component
        // specular highLights: Cs= Msp Lsp pow(n dot h, Msh)
            vec4 Cs = material.specular * lightSource.specular * pow(max(nDotH,0), material.shininess);
            //return Cs;

        // TODO (4.9): compute brdf value f = Ca + Cd + Cs
            vec4  f = Ca + Cd + Cs;

	 return f;
       }
       );

const std::string flatVertexShader=

  GLSL(
       
       uniform mat4 modelViewProjectionMatrix;
       uniform mat4 modelViewMatrix;
       
       attribute vec4 position;
       
       varying vec4 vertex;
       
       void main(void){
	 
	 vertex= modelViewMatrix*position;
	 
	 gl_Position= modelViewProjectionMatrix*position;
       }
       );


const std::string flatGeometryShader=

  GLSL(

       //       vec4 blinnPhongReflection(vec4 position, vec3 normal);

       varying in vec4 vertex[3];

       varying out vec4 color;

       void main(void) {

	 // compute face normal\n"
	 vec3 edge1= vec3(vertex[0] - vertex[1]);
	 vec3 edge2= vec3(vertex[1] - vertex[2]);
	 edge1= normalize(edge1);
	 edge2= normalize(edge2);
	 vec3 faceNormal= cross(edge1, edge2);
	 faceNormal= normalize(faceNormal);

	 vec4 centerOfGravity= 1.0/3.0*(vertex[0]+vertex[1]+vertex[2]);

	 color= blinnPhongReflection(centerOfGravity, faceNormal);

	 gl_Position= gl_PositionIn[0];
	 EmitVertex();

	 gl_Position= gl_PositionIn[1];
	 EmitVertex();

	 gl_Position= gl_PositionIn[2];
	 EmitVertex();

	 EndPrimitive();
       }
       );
 
const std::string flatFragmentShader= 
  
  GLSL(
       varying vec4 color;
       
       void main(void){
	 
	 gl_FragColor= color;
       }
       );


const std::string gouraudVertexShader=
  
  GLSL(
       uniform mat4 modelViewProjectionMatrix;
       uniform mat4 modelViewMatrix;
       uniform mat3 normalMatrix;
       
       attribute vec4 position;
       attribute vec3 normal;
       
       varying vec4 color;
       
       void main(){
	 
	 color= blinnPhongReflection(modelViewMatrix*position, normalize(normalMatrix*normal));
	 
	 // transform to clip space
	 gl_Position = modelViewProjectionMatrix*position;
       }
       );

const std::string gouraudFragmentShader=

  GLSL(
     
       varying vec4 color;
	 
       void main(void){
	   
	 gl_FragColor= color;			
       }
       );


const std::string phongVertexShader=
  
  GLSL(
       uniform mat4 modelViewProjectionMatrix;
       uniform mat4 modelViewMatrix;
       uniform mat3 normalMatrix;
       
       attribute vec4 position;
       attribute vec3 normal;
       
       varying vec4 fragmentPosition;
       varying vec3 fragmentNormal;
       
       void main(){
	 
	 // transform to view space
	 fragmentPosition= modelViewMatrix*position;
	 fragmentNormal= normalize(normalMatrix*normal);
	 
	 // transform to clip space
	 gl_Position = modelViewProjectionMatrix*position;
       }
       );

const std::string phongFragmentShader=

  GLSL(
       varying vec4 fragmentPosition;
       varying vec3 fragmentNormal;

       void main(void){

	 gl_FragColor= blinnPhongReflection(fragmentPosition, normalize(fragmentNormal));
       }
       );

#else // EXTRA

const std::string version= "#version 150\n";
const std::string geometry_shader4= "";

const std::string faceNormalVizVertexShader= 

  GLSL(
    
       uniform mat4 modelViewProjectionMatrix;
    
       in vec4 position;
       out vec4 vertex;
    
       void main(){
    
	 vertex= position;	
	 gl_Position= modelViewProjectionMatrix*position;
       }
       );
       
const std::string faceNormalVizGeometryShader= 

  GLSL(

       layout(triangles) in;
       layout(triangle_strip, max_vertices = 3) out;
       
       in vec4 vertex[3];
       
       out vec4 color;

       void main(void){

	 // compute face normal
	 vec3 edge1= vec3(vertex[1]-vertex[0]);
	 vec3 edge2= vec3(vertex[2]-vertex[0]);
	 vec3 faceNormal= cross(edge1, edge2);
	 faceNormal= normalize(faceNormal);
	 // color can't be negative
	 faceNormal.xyz*=0.5;
	 faceNormal.xyz+=0.5;

	 color= vec4(faceNormal, 1);
	 gl_Position = gl_in[0].gl_Position;
	 EmitVertex();

	 color= vec4(faceNormal, 1);
	 gl_Position = gl_in[1].gl_Position;
	 EmitVertex();

	 color= vec4(faceNormal, 1);
	 gl_Position = gl_in[2].gl_Position;
	 EmitVertex();

	 EndPrimitive();
       }
       );

const std::string faceNormalVizFragmentShader=
  
  GLSL(

       out vec4 pixel;
       in vec4 color;

       void main(void){

	 pixel= color;			
       }
       );

const std::string lightDirectionVertexShader=
       
  GLSL(
	    
       uniform mat4 modelViewProjectionMatrix;
       uniform mat4 modelViewMatrix;

       in vec4 position;
       out vec4 vertex;

       void main(){

	 vertex= modelViewMatrix*position;
	 gl_Position= modelViewProjectionMatrix*position;
       }
       );

const std::string lightDirectionFragmentShader=

  GLSL(

       uniform vec4 lightPosition;

       in vec4 vertex;

       out vec4 pixel;

       void main(void){

	 vec3 lightDirection= vec3(lightPosition-vertex);

	 vec4 color= vec4(normalize(lightDirection.xyz),1);
	 // color can't be negative
	 color.xyz*= 0.5;
	 color.xyz+= 0.5;

	 pixel= color;
       }
       );

const std::string blinnPhongReflectionShader=

  GLSL(

       struct LightSource{
	 //position in view space
	 vec4 position;
	 // ambient color
	 vec4 ambient;
	 // diffuse color
	 vec4 diffuse;
	 // specular color
	 vec4 specular;
       };

       struct Material{
	 // ambient color
	 vec4 ambient;
	 // diffuse color
	 vec4 diffuse;
	 // specular color
	 vec4 specular;
	 // shininess
	 float shininess;
       };

       uniform LightSource lightSource;
       uniform Material material;

       // position and normal in view space
       vec4 blinnPhongReflection(vec4 position, vec3 normal){

	 // compute the light and view direction
	 vec3 lightDirection= normalize(lightSource.position.xyz-position.xyz);
	 // we are in eye coordinates, so eye position is (0,0,0)
	 vec3 viewDirection= normalize(-position.xyz);

	 // ambient lighting
	 vec4 ambient= lightSource.ambient*material.ambient;

	 // diffuse lighting
	 float nDotL= max(dot(normal,lightDirection),0.0);
	 vec4 diffuse= nDotL*lightSource.diffuse*material.diffuse;

	 // compute half vector
	 vec3 halfVector= normalize(lightDirection + viewDirection);
	 // specular highLights
	 float nDotH= max(dot(normal, halfVector), 0.0);
	 vec4 specular= material.specular*lightSource.specular*pow(nDotH, material.shininess);

	 // compute brdf value f = ca + cd + cs 
	 return ambient + diffuse + specular;
       }
       );
       
const std::string flatVertexShader=
       
  GLSL(
	    
       uniform mat4 modelViewProjectionMatrix;
       uniform mat4 modelViewMatrix;
	    
       in vec4 position;
	    
       out vec4 vertex;
	    
       void main(void){
	    
	 vertex= modelViewMatrix*position;
	    
	 gl_Position= modelViewProjectionMatrix*position;
       }
       );
	    
const std::string flatGeometryShader=
	    
  GLSL(
		 
       layout(triangles) in;
       layout(triangle_strip, max_vertices = 3) out;
	    
       vec4 blinnPhongReflection(vec4 position, vec3 normal);

       in vec4 vertex[3];

       out vec4 color;

       void main(void) {

	 // compute face normal
	 vec3 edge1= vec3(vertex[0] - vertex[1]);
	 vec3 edge2= vec3(vertex[1] - vertex[2]);
	 edge1= normalize(edge1);
	 edge2= normalize(edge2);
	 vec3 faceNormal= cross(edge1, edge2);
	 faceNormal= normalize(faceNormal);

	 vec4 centerOfGravity= 1.0/3.0*(vertex[0]+vertex[1]+vertex[2]);

	 color= blinnPhongReflection(centerOfGravity, faceNormal);

	 gl_Position= gl_in[0].gl_Position;
	 EmitVertex();

	 gl_Position= gl_in[1].gl_Position;
	 EmitVertex();

	 gl_Position= gl_in[2].gl_Position;
	 EmitVertex();

	 EndPrimitive();
       }
       );

const std::string flatFragmentShader= 

  GLSL(

       in vec4 color;

       out vec4 pixel;

       void main(void){

	 pixel= color;
       }
       );

const std::string gouraudVertexShader=

  GLSL(

       uniform mat4 modelViewProjectionMatrix;
       uniform mat4 modelViewMatrix;
       uniform mat3 normalMatrix;

       in vec4 position;
       in vec3 normal;

       out vec4 color;

       vec4 blinnPhongReflection(vec4 position, vec3 normal);

       void main(){

	 color= blinnPhongReflection(modelViewMatrix*position, normalize(normalMatrix*normal));

	 // transform to clip space
	 gl_Position = modelViewProjectionMatrix*position;
       }
       );
const std::string gouraudFragmentShader=

  GLSL(

       out vec4 pixel;
       in vec4 color;

       void main(void){

	 pixel= color;			
       }
       );


const std::string phongVertexShader=

  GLSL(

       uniform mat4 modelViewProjectionMatrix;
       uniform mat4 modelViewMatrix;
       uniform mat3 normalMatrix;

       in vec4 position;
       in vec3 normal;

       out vec4 fragmentPosition;
       out vec3 fragmentNormal;

       void main(){

	 // transform to view space
	 fragmentPosition= modelViewMatrix*position;
	 fragmentNormal= normalize(normalMatrix*normal);

	 // transform to clip space
	 gl_Position = modelViewProjectionMatrix*position;
       }
       );

const std::string phongFragmentShader=
  GLSL(
     
       in vec4 fragmentPosition;
       in vec3 fragmentNormal;

       out vec4 pixel;

       vec4 blinnPhongReflection(vec4 position, vec3 normal);

       void main(void){

	 pixel= blinnPhongReflection(fragmentPosition, normalize(fragmentNormal));		
       }
       );

#endif // EXTRA
