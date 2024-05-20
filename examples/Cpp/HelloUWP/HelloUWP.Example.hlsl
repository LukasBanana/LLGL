struct InputVS {
	float2 position : POSITION;
	float4 color    : COLOR;
};
struct OutputVS {
	float4 position : SV_Position;
	float4 color    : COLOR;
};
void VS(InputVS inp, out OutputVS outp) {
	outp.position = float4(inp.position, 0, 1);
	outp.color = inp.color;
}
float4 PS(OutputVS inp) : SV_Target {
	return inp.color;
};

