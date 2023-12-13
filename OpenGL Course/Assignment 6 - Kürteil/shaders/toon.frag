uniform vec4 color;
uniform bool texturing;
uniform sampler2D texture;

varying float shade;
varying vec2 t;

void main(void){

  vec4 pixel= color;

  if(texturing){
    vec4 texel= texture2D(texture, t);

    pixel= (texel.r==texel.g&&texel.r==texel.b) ?
    color*texel :
    texel;
  }

  if( shade<0.25) pixel.xyz*= 1.5;
  else if(shade<0.75) pixel.xyz*= 0.66;

  gl_FragColor= pixel;
}
