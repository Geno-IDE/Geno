R"(#version 330

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 TexCoords[4];
layout (location = 5) in vec2 PositionOffset;
layout (location = 6) in vec4 Colors;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec4 Color;
layout (location = 2) out float CharSize;

uniform float Size = 150;
uniform mat4 Projection;

void main() {
    gl_Position = Projection * vec4((Position + PositionOffset)  * Size, 0, 1);

    TexCoord = TexCoords[gl_VertexID];
    Color = Colors;
    CharSize = Size;
}

)"