uniform mat4 transformation;
uniform vec4 lightPosition;

attribute vec4 position;
attribute vec3 normal;
attribute vec2 texCoord;
       
varying float shade;
varying vec2 t;
       
void main(){

  t= texCoord;

  vec3 lightDirection= normalize(lightPosition.xyz-position.xyz);

  shade= dot(normal, lightDirection);

  gl_Position= transformation*position;
}
