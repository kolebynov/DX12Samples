struct PSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

PSInput VSMain(float4 Position : POSITION, float4 Color : COLOR)
{
	PSInput psInput;

	psInput.Position = Position;
	psInput.Color = Color;

	return psInput;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.Color;
}