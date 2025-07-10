#version 410 core

layout (location = 0) in float aColor;

uniform mat4 projection;
uniform vec2 gridSize;

out vec4 OutColor;

const vec2 VertexPositions[4] = vec2[4](
  vec2(0.0, 0.0),
  vec2(0.8, 0.0),
  vec2(0.8, 0.8),
  vec2(0.0, 0.8)
);

void main()
{
  int i = int(aColor);
  switch (i)
  {
    case 1:
      OutColor = vec4(1, 1, 1, 1);
      break;
    default:
      OutColor = vec4(0, 0.1, 0.8, 1) * aColor / -20.0;
      break;
  }

  vec2 translation;
  translation.x = mod(gl_InstanceID, gridSize.x);
  translation.y = int(gl_InstanceID) / int(gridSize.x);
  vec2 position = VertexPositions[gl_VertexID] + translation;
  gl_Position = projection * vec4(position, 0, 1);
}
