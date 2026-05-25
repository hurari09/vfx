#version 330 core
layout (location = 0) in vec3 aVel;
layout (location = 1) in vec3 aRGB;

out vec4 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 gravity;
uniform float lifetime;
uniform float time;

void main()
{
	vec3 pos = vec3(0.0);
	float alpha = 1.0;
	if (time < lifetime){
		pos = aVel * time + gravity * time * time; 
		alpha = 1.0 - time/lifetime;
	}
	else
		alpha = 0.0;
	Color = vec4(aRGB, alpha);
	gl_Position = projection * view * model * vec4(pos, 1.0);
}