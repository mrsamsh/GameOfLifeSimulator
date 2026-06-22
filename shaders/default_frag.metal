#include <metal_stdlib>
using namespace metal;

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
  {0.0, 0.025,  0.2,   1},
  {1,   1,      1,     1}
};

struct VertexOut
{
  float4 position [[position]];
  float2 texcoord;
  float2 gridSize;
};

fragment float4 FSmain(VertexOut in [[stage_in]],
                        constant int8_t* indices [[buffer(0)]])
{
  float2 local = fract(in.texcoord);
  float margin = 0.1;
  if (local.x < margin || local.x > 1.0 - margin || local.y < margin || local.y > 1.0 - margin)
    return float4(0, 0.0125, 0.1, 1);
  int i = floor(in.texcoord.x) + floor(in.texcoord.y) * in.gridSize.x;
  return palette[indices[i]];
}

