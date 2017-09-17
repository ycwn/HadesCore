
#shader triangle
#stage  500

#render    framebuffer
#vertex    V2C3
#primitive triangles


#section
#version 450
#extension GL_ARB_separate_shader_objects: enable


#section .vert


layout(location = 0) in vec2 inPosition;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor   = inColor;
}


#section .frag

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}


