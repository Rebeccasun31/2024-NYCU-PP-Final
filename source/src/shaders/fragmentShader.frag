#version 330 core
out vec4 FragColor;

in vec2 TexCoord; 

uniform sampler2D ourTexture;
uniform vec3 ourColor;

void main()
{
    // TODO: Implement Rainbow Effect
	//   1. Retrieve the color from the texture at texCoord.
	//   2. Set FragColor to be the dot product of the color and rainbowColor
	//   Note: Ensure FragColor is appropriately set for both rainbow and normal cases.

	if (ourColor == vec3(0.0f, 0.0f, 0.0f))
		FragColor = texture2D(ourTexture, TexCoord);
	else
		FragColor = vec4(ourColor, 1);
} 
