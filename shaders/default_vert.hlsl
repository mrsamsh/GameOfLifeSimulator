struct UniformBufferObject
{
  float4x4 projection;
  float2   windowSize;
  float2   gridSize;
  float    cellSide;
};

cbuffer UBO : register(b0, space0)
{
  UniformBufferObject ubo;
};

struct Output
{
  float4 position  : SV_Position;
  float2 texcoord  : TEXCOORD0;
  float2 gridSize  : TEXCOORD1;
};

static const float2 VertexPositions[6] = {
  float2(0.0, 0.0),
  float2(1.0, 0.0),
  float2(1.0, 1.0),
  float2(0.0, 1.0),
  float2(0.0, 0.0),
  float2(1.0, 1.0)
};

Output VSmain(uint vid : SV_VertexID)
{
  Output output;
  output.position = mul(ubo.projection, float4(VertexPositions[vid] * ubo.windowSize, 0, 1));
  output.texcoord = VertexPositions[vid] * ubo.gridSize;
  output.gridSize = ubo.gridSize;
  return output;
}
