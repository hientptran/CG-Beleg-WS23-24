// UMWANDLUNG DER POSITIONEN VON OBJEKTEN IN 3D-ZEICHENKOORDINATEN

// Values that stay constant for the whole mesh.
//uniform mat4 MVP;
uniform mat4 transformation;

attribute vec4 position;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  transformation * position;
	
}

