

cbuffer ConstantBufferSprite : register( b0 )
{
	float			render_pos_x;
	float			render_pos_y;
	float			render_width;
	float			render_height;

	float			screen_width;
	float			screen_height;
	float			render_z;
	float			fAlpha;
		
	float4			diffuseColor;
}

Texture2D		texDiffuse		: register( t0 );
SamplerState	samplerDiffuse	: register( s0 );

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4		Pos		 : POSITION;
	float2		TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Pos		 : SV_POSITION;
    float4 Color	 : COLOR;
	float2 TexCoord	 : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------




PS_INPUT vsDefault( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	
	// position
	float	scale_x = render_width / screen_width;
	float	scale_y = render_height / screen_height;

	float	x_offset = render_pos_x / screen_width;
	float	y_offset = render_pos_y / screen_height;

	
	float2	pos = input.Pos.xy * float2(scale_x,scale_y) + float2(x_offset,y_offset);
	output.Pos = float4(pos.x*2-1 ,(1-pos.y)*2-1,render_z,1);

	//output.Pos = input.Pos * float4(w,h,render_z,1);
	output.TexCoord = float4(input.TexCoord,0,0);
	output.Color = diffuseColor;
	    
    return output;
}

float4 psDefault( PS_INPUT input) : SV_Target
{
	float4	texColor = texDiffuse.Sample( samplerDiffuse, input.TexCoord);

	//float	Y = texColor.r;
	//float	U = texColor.g;
	//float	V = texColor.b;

	/*
	BYTE R = (BYTE)( (float)Y + 1.402f*(float)(V-128));
	BYTE G = (BYTE)( (float)Y - 0.344f*(float)(U-128) - 0.714f*(float)(V-128) );
	BYTE B = (BYTE)( (float)Y + 1.772f*(float)(U-128) );
	*/

	float	Y = 1.1643 * texColor.r;
	float	U = texColor.g - 0.5;
	float	V = texColor.b - 0.5;

	float3	color;
	color.r = Y + 1.5958 * V;
	color.g = Y - 0.39173 * U - 0.81290 * V;
	color.b = Y + 2.017 * U;
	
	float4	outColor = input.Color * float4(color,1);
	//float4	outColor = float4(Y,Y,Y,1);
	
	
//	clip(outColor.a - 0.003f);
//	clip(texColor.a - 0.003f);
//	clip(texColor.a);

	return outColor;
}
