static const float4 palette[22] = {
  float4(0.0, 0.100,  0.800, 1),
  float4(0.0, 0.095,  0.760, 1),
  float4(0.0, 0.090,  0.720, 1),
  float4(0.0, 0.085,  0.680, 1),
  float4(0.0, 0.080,  0.640, 1),
  float4(0.0, 0.075,  0.600, 1),
  float4(0.0, 0.070,  0.560, 1),
  float4(0.0, 0.065,  0.520, 1),
  float4(0.0, 0.060,  0.480, 1),
  float4(0.0, 0.055,  0.440, 1),
  float4(0.0, 0.050,  0.400, 1),
  float4(0.0, 0.045,  0.360, 1),
  float4(0.0, 0.040,  0.320, 1),
  float4(0.0, 0.035,  0.280, 1),
  float4(0.0, 0.030,  0.240, 1),
  float4(0.0, 0.025,  0.200, 1),
  float4(0.0, 0.020,  0.160, 1),
  float4(0.0, 0.015,  0.120, 1),
  float4(0.0, 0.010,  0.080, 1),
  float4(0.0, 0.005,  0.040, 1),
  float4(0.0, 0.025,  0.2,   1),
  float4(1,   1,      1,     1)
};

ByteAddressBuffer indices : register(t0, space0);

struct Input
{
  float4 position  : SV_Position;
  float2 texcoord  : TEXCOORD0;
  float2 gridSize  : TEXCOORD1;
};

float4 FSmain(Input input) : SV_Target0
{
  float2 local = frac(input.texcoord);
  float margin = 0.1;
  if (local.x < margin || local.x > 1.0 - margin || local.y < margin || local.y > 1.0 - margin)
    return float4(0, 0.0125, 0.1, 1);
  int i = floor(input.texcoord.x) + floor(input.texcoord.y) * input.gridSize.x;
  int current_value = indices.Load<int>(i);
  return palette[current_value + 20];
}
