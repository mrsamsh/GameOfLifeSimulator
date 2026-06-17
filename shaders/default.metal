#include <metal_stdlib>
using namespace metal;

struct UniformBufferObject
{
  float4x4 projection;
  float2   gridSize;
  float    cellSide;
};

struct VertexOut
{
  float4 position  [[position]];
  float4 color;
};

constant float2 VertexPositions[6] = {
  {0.0, 0.0},
  {0.8, 0.0},
  {0.8, 0.8},
  {0.0, 0.8},
  {0.0, 0.0},
  {0.8, 0.8}
};

constant float4 palette[] = {
  {0.0, 0.100,  0.800, 1},
  {0.0, 0.095,  0.760, 1},
  {0.0, 0.090,  0.720, 1},
  {0.0, 0.085,  0.680, 1},
  {0.0, 0.080,  0.640, 1},
  {0.0, 0.075,  0.600, 1},
  {0.0, 0.070,  0.560, 1},
  {0.0, 0.065,  0.520, 1},
  {0.0, 0.060,  0.480, 1},
  {0.0, 0.055,  0.440, 1},
  {0.0, 0.050,  0.400, 1},
  {0.0, 0.045,  0.360, 1},
  {0.0, 0.040,  0.320, 1},
  {0.0, 0.035,  0.280, 1},
  {0.0, 0.030,  0.240, 1},
  {0.0, 0.025,  0.200, 1},
  {0.0, 0.020,  0.160, 1},
  {0.0, 0.015,  0.120, 1},
  {0.0, 0.010,  0.080, 1},
  {0.0, 0.005,  0.040, 1},
  {0.0, 0.0125, 0.1,   1},
  {1,   1,      1,     1}
};

vertex VertexOut vertexShader(uint vertexID [[vertex_id]],
                              uint instanceID [[instance_id]],
                              constant UniformBufferObject& ubo [[buffer(0)]],
                              constant int8_t* cells [[buffer(1)]],
                              constant float2*  translations[[buffer(2)]]
                              )
{
  VertexOut out;
  out.color = palette[cells[instanceID] + 20];
  float2 position = (VertexPositions[vertexID] * ubo.cellSide + translations[instanceID] * ubo.cellSide);
  out.position = ubo.projection * float4(position, 0, 1);
  return out;
}

fragment float4 fragmentShader(VertexOut in [[stage_in]])
{
  return in.color;
}
