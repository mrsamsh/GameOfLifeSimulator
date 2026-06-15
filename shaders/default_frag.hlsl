struct VertexOut
{
  float4 position      : SV_Position;
  float4 color         : TEXCOORD0;
};

float4 FSmain(VertexOut input) : SV_Target0
{
  return input.color;
}
