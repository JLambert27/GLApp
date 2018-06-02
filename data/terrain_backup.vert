#version 150 core
// vertex shader for simple terrain demo

// per-frame data
layout(std140)                  // standard layout
uniform SceneData {             // like a class name
    mat4 modelViewMatrix, modelViewInverse;
    mat4 projectionMatrix, projectionInverse;
	mat4 TBN, TBNInverse;
    vec4 fog;
	vec4 normRelief;
};

//shader settings
uniform sampler2D normalTexture;

// per-vertex input
in vec3 vPosition;
in vec3 vNormal; 
in vec3 vNormalMap;
in vec2 vUV;

// output to fragment shader (view space)
out vec3 normal;
out vec3 normalMap;
out vec2 texcoord;
out vec4 position;
out mat4 TBNMat;

void main() {
    position = modelViewMatrix * vec4(vPosition, 1);
	if(false)//(normRelief.x != 0)			
	{
		//use normal map for normal instead of vNormal
		//need to convert normal map into view space using 

		Mat4f TBNMat; 
		vec3 normTemp = normalize(vNormal * mat3(modelViewInverse));
		TBNMat[2][0] = vNormal.x; TBNMat[2][1] = vNormal.y; TBNMat[2][2] = vNormal.z;
		Vec4f bitangent = dot(vNormal, )

		normal = normalize(texture(normalTexture, vUV).xyz);				//CALCULATE TBN

		//normal.x = 1; normal.y = .9; normal.z = 1;
		//normal = normalize(normal);
	}
	else //normRelief.x == 0
	{
		normal = normalize(vNormal * mat3(modelViewInverse));
	}
	normalMap = vNormalMap;
    texcoord = vUV;
    gl_Position = projectionMatrix * position;
}
