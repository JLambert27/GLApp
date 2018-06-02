#version 150 core
// vertex shader for simple terrain demo

// per-frame data
layout(std140)                  // standard layout
uniform SceneData {             // like a class name
    mat4 modelViewMatrix, modelViewInverse;
    mat4 projectionMatrix, projectionInverse;
    vec4 fog;
	vec4 normRelief;
};

// per-vertex input
in vec3 vPosition;
in vec3 vNormal;
in vec3 vNormalMap;
in vec2 vUV;

// output to fragment shader (view space)
out vec3 normal;
out vec2 texcoord;
out vec3 normalMap;
out vec4 position;
out mat3 TBNMat;

void main() {
    position = modelViewMatrix * vec4(vPosition, 1);
    normal = normalize(vNormal * mat3(modelViewInverse));

	//construct TBN Matrix
	mat3 TBNMat; 
	TBNMat[2][0] = normal.x; TBNMat[2][1] = normal.y; TBNMat[2][2] = normal.z;
//	TBNMat[1][0] = 

	normalMap = vNormalMap * TBNMat; 
    texcoord = vUV;
    gl_Position = projectionMatrix * position;
}
