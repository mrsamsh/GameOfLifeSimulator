#include <metal_stdlib>
using namespace metal;

struct UniformBufferObject
{
  float4x4 projection;
  float2   windowSize;
  float2   gridSize;
  float    cellSide;
};

struct VertexOut
{
  float4 position [[position]];
  float2 texcoord;
  float2 gridSize;
};

constant float2 VertexPositions[6] = {
  {0.0, 0.0},
  {1.0, 0.0},
  {1.0, 1.0},
  {0.0, 1.0},
  {0.0, 0.0},
  {1.0, 1.0}
};

vertex VertexOut VSmain(uint vertexID [[vertex_id]],
                        constant UniformBufferObject& ubo [[buffer(0)]])
{
  VertexOut out;
  out.position = ubo.projection * float4(VertexPositions[vertexID] * ubo.windowSize, 0, 1);
  out.texcoord = VertexPositions[vertexID] * ubo.gridSize;
  out.gridSize = ubo.gridSize;
  return out;
}

