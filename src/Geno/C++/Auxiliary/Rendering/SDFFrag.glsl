R"(#version 330

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec4 Color;
layout (location = 2) in float CharSize;

layout (location = 0) out vec4 glColor;

uniform sampler2D Atlas;

float Median(vec3 Vec) {
    return max(min(Vec.x, Vec.y), min(max(Vec.x, Vec.y), Vec.z));
} 

void main() {
    float SignedDistance = Median(texture(Atlas, TexCoord).xyz);
    float ScreenPixelDistance = ((CharSize / 32.0) * 2.0)*(SignedDistance - 0.5);
    float Opacity = clamp(ScreenPixelDistance+0.5, 0.0, 1.0);
    glColor = mix(vec4(0, 0, 0, 0), Color, Opacity);
}

)"