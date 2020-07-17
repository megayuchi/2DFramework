#define		DEFAULT_ID		0.025f
#define		WATER_ID		0.025f
#define		DYNAMIC_ID		0.0275f
#define		HFIELD_ID		0.200f
#define		LMMESH_ID		0.225f
#define		SKY_ID			0.75f

#define MAX_CASCADE_NUM		4


struct ATT_LIGHT
{
// 16bytes align
	float4		Pos;
	float		Rs;
	float		RsRs;
	float		RcpRs;
	float		RcpRsRs;
	float4		ColorCenter;
	float4		ColorSide;
};


struct CASCADE_CONSTNAT
{
	// 16 bytes align
	float		Dist;
	float		BiasDynamic;
	float		BiasStatic;
	float		Reserved0;
};


struct PROJ_CONSTANT
{
	float		fNear;
	float		fFar;
	float		fFarRcp;
	float		Reserved1;
};
struct LIGHT_CUBE_CONST
{
	float4			ColorPerAxis[6];	
};

// Vertex Shader Constant ///////////////////////////
cbuffer ConstantBufferDefault : register( b0 )
{
	matrix				matWorld;
	matrix				matView;
	matrix				matProj;
	matrix				matWorldInv;
	matrix				matViewProj;
	matrix				matWorldView;
	matrix				matWorldViewProj;
	matrix				matShadowViewProjCascade[MAX_CASCADE_NUM];
	CASCADE_CONSTNAT	CascadeConst[MAX_CASCADE_NUM];
	float4				LightMapConst;
	float4				Lightdir;
	float4				LightColor;
	LIGHT_CUBE_CONST	LightCube;
	float4				ShadowLightDirInv;
	float4				ClipPlane;		// a,b,c,d = x,y,z,w
	PROJ_CONSTANT		ProjConstant;
	int					iAttLightNum;
	int					iReserved1;
	int					iReserved2;
	int					iReserved3;
	float4				WorldMinMax[2];
	float4				WorldSize;
	ATT_LIGHT			AttLight[8];
	float4				PublicConst[4];
}


// Pixel Shader Constant ///////////////////////////
cbuffer ConstantBufferAlphaThresold : register( b1 )
{
	float4		AlphaThresold;
	float4		ColorThresold;
	float4		DefaultColor;
}

cbuffer ConstantBufferMaterial : register( b2 )
{
	float4		MtlDiffuse;
	float4		MtlAmbient;
	float4		MtlDiffuseAdd;		// 머리카락 도색등 특수한 경우 강조해주기 위해 픽셀 쉐이더로 넘어간 이후 사용
	float4		MtlToneColor;		// 최종렌더링 결과물에 곱해줄 상수, 밝기가 어두울때 주로 사용
	float4		MinNdotL;			// min (r,g,b,a = VertexLighting시 N dot L의 최저값)
}

cbuffer ConstantBufferWaterBottom : register( b3 )
{
	float		Height;
	float		RcpFadeDistance;
	float		FadeDistance;
	float		Reserved;
}



struct VS_INPUT_XYZW
{
    float4		Pos		 : POSITION;
};

struct VS_INPUT_VL
{
    float4		Pos		 : POSITION;
	float3		Normal	 : NORMAL;
	float3		Tangent	 : TANGENT;
	float2		TexCoord : TEXCOORD0;

};
struct VS_INPUT_VL_PHYSIQUE
{	
	float4		Pos				: POSITION;
	float3		Normal			: NORMAL;
	float3		Tangent			: TANGENT;

	uint4		BlendIndex		: BLENDINDICES;
	float4		BlendWeight		: BLENDWEIGHTS;

	float2		TexCoord		: TEXCOORD0;

};
struct PS_INPUT_COLOR
{
    float4 Pos				: SV_POSITION;
};

struct PS_INPUT_DEPTH
{
    float4	Pos			: SV_POSITION;
	float	Depth	    : ZDEPTH;
};



struct PS_OUT
{
	float4 Color0 : SV_Target0; // pixel color
	float4 Color1 : SV_Target1; // normal : depth
	float4 Color2 : SV_Target2; // depth
	
};


PS_INPUT_COLOR vsXYZ( VS_INPUT_XYZW input )
{
    PS_INPUT_COLOR output = (PS_INPUT_COLOR)0;

	output.Pos = mul( input.Pos, matWorldViewProj );
    
    return output;
}
float4 psColor( PS_INPUT_COLOR input) : SV_Target
{
	float4	outColor = DefaultColor;

	return outColor;
}

PS_INPUT_DEPTH vsDepthDist( VS_INPUT_XYZW input )
{
    PS_INPUT_DEPTH output = (PS_INPUT_DEPTH)0;

	output.Pos = mul( input.Pos, matWorldViewProj );
	//output.Depth = output.Pos.z / output.Pos.w;
	//output.Depth = saturate(output.Pos.w * (1.0f / 10000.0f));
	output.Depth = output.Pos.w * ProjConstant.fFarRcp;
	
    
    return output;
}

float4 psDepthDist(PS_INPUT_DEPTH input) : SV_Target
{
	float4	outColor = float4(input.Depth,input.Depth,input.Depth,1);
	//float4	outColor = float4(input.Depth,1,1,1);

	return outColor;
}



// 다이나믹라이트 처리
float3 CalcAttLightColor(float3 Pos,int AttLightNum)
{
	float3		LightColorSum = 0;
	
	for (int i=0; i<AttLightNum; i++)
	{
		float3		LightVec = (AttLight[i].Pos.xyz - Pos.xyz);
		float		LightVecDot = dot(LightVec,LightVec);
		float		Dist = sqrt(LightVecDot);
		float		RsSubDist = AttLight[i].Rs - Dist;
		
		if (RsSubDist < 0.0f)
		{
			continue;
		}
		float	NrmDist = (Dist / AttLight[i].Rs);
		float	FallOff = (RsSubDist*RsSubDist) * AttLight[i].RcpRsRs;
		float3	LightColor = lerp(AttLight[i].ColorCenter.rgb,AttLight[i].ColorSide.rgb,NrmDist) * FallOff;
	
		LightColorSum += LightColor;
	}
	return LightColorSum;
}
float3 CalcAttLightColorWithNdotL(float3 Pos,float NdotL[8],int AttLightNum)
{

	float3		LightColorSum = 0;

	for (int i=0; i<iAttLightNum; i++)
	{
		if (NdotL[i] <= 0.0f)
		{
			continue;
		}

		float3		LightVec = (AttLight[i].Pos.xyz - Pos.xyz);
		float		LightVecDot = dot(LightVec,LightVec);
		float		Dist = sqrt(LightVecDot);
		float		RsSubDist = AttLight[i].Rs - Dist;
		
		if (RsSubDist < 0.0f)
		{
			continue;
		}
		float	NrmDist = (Dist / AttLight[i].Rs);
		float	FallOff = (RsSubDist*RsSubDist) * AttLight[i].RcpRsRs;
		float3	LightColor = lerp(AttLight[i].ColorCenter.rgb,AttLight[i].ColorSide.rgb,NrmDist) * FallOff;
	
		LightColorSum += LightColor;
	}
	return LightColorSum;
}

void CalcIndex_Dynamic(out float OutIndex,out float OutBias,in float Dist)
{
	uint	index = MAX_CASCADE_NUM-1;

	for (uint i=0; i<MAX_CASCADE_NUM; i++)
	{
		if (Dist <= CascadeConst[i].Dist)
		{
			index = i;
			break;
		}
	}


	OutIndex = index;
	OutBias = CascadeConst[index].BiasDynamic;
	
}
void CalcIndex_Static(out float OutIndex,out float OutBias,in float Dist)
{
	uint	index = MAX_CASCADE_NUM-1;

	for (uint i=0; i<MAX_CASCADE_NUM; i++)
	{
		if (Dist <= CascadeConst[i].Dist)
		{
			index = i;
			break;
		}
	}


	OutIndex = index;
	OutBias = CascadeConst[index].BiasStatic;
	
}
float3 CalcShadowColor4x4_Dynamic(Texture2D texShadowMap,SamplerComparisonState samplerComp,float4 PosWorld,float Dist,out uint OutIndex)
{
	float3	shadowColor = float3(1,1,1);

	uint	index;
	float	Bias;
	CalcIndex_Dynamic(index,Bias,Dist);
	
	OutIndex = index;
		
	float4	PosShadowSpace = mul(PosWorld,matShadowViewProjCascade[index]);
	float4	texCoord = PosShadowSpace / PosShadowSpace.w;

	if (texCoord.x < 0.0f || texCoord.x > 1.0f)
		return shadowColor;

	 if (texCoord.y < 0.0f || texCoord.y > 1.0f)
		 return shadowColor;
	 
	 if (texCoord.z < 0.0f || texCoord.z > 1.0f)
		 return shadowColor;

	//texCoord.x = (1.0 / MAX_CASCADE_NUM)*(index + texCoord.x);

	// ((index*1024) + (texCoord.x*1024)) / (1024*4)
	// ((index*1024) / (1024*4)) + ((texCoord.x*1024) / (1024*4))
	// (index / 4) + (texcoord.x / 4)
	texCoord.x = ((float)index + texCoord.x) / (float)MAX_CASCADE_NUM;

	//texCoord.z -= ShadowMapBias;
	texCoord.z -= Bias;
	//texCoord.x = (index * (1.0 / MAX_CASCADE_NUM)) + PosShadowSpace.x*(1.0 / MAX_CASCADE_NUM) ;
		
	//PosShadowSpace.z -= ShadowMapBias;
	

	float	litSum = 0;
	float	shadowPixel;
	/*
	for (float y=-1.0; y<=1.0; y += 1.0)
	{
		for (float x=-1.0; x<=1.0; x += 1.0)
		{
			//float4	texDepth = texShadowMap.Sample(samplerBorder,texCoord.xy + float2(x/1024.0f,y/1024.0f));
			//litSum += (PosShadowSpace.z - texDepth.r  <= 0.0f );		
			
			litSum += texShadowMap.SampleCmpLevelZero(samplerComp,texCoord.xy + float2(x/(1024.0f*MAX_CASCADE_NUM),y/1024.0f),texCoord.z );
		}
	}
	float	shadowColor = litSum / 9.0f;
	*/
	
	for (float y=-1.5; y<=1.5; y += 1.0)
	{
		for (float x=-1.5; x<=1.5; x += 1.0)
		{
//			float4	texDepth = texShadowMap.Sample(samplerBorder,texCoord.xy + float2(x/1024.0f,y/1024.0f));
//			litSum += (PosShadowSpace.z - texDepth.r  <= 0.0f );		
			
			litSum += texShadowMap.SampleCmpLevelZero(samplerComp,texCoord.xy + float2(x/(1024.0f*MAX_CASCADE_NUM),y/1024.0f),texCoord.z );
		}
	}
	float	shadowValue = litSum / 16.0f;
	
	shadowColor.r = shadowValue * 0.25f + 0.75f;
	shadowColor.g = shadowValue * 0.4f + 0.6f;
	shadowColor.b = shadowValue * 0.4f + 0.6f;
	//shadowColor = shadowColor * 0.5f + 0.5f;

	return shadowColor;
}


float CalcShadowColor4x4_Static(Texture2D texShadowMap,SamplerComparisonState samplerComp,float4 PosWorld,float Dist,out uint OutIndex)
{
	float	shadowColor = 1.0f;

	uint	index;
	float	Bias;
	CalcIndex_Static(index,Bias,Dist);
	
	OutIndex = index;

	float4	PosShadowSpace = mul(PosWorld,matShadowViewProjCascade[index]);
	float4	texCoord = PosShadowSpace / PosShadowSpace.w;

	
	if (texCoord.x < 0.0f || texCoord.x > 1.0f)
		return shadowColor;

	 if (texCoord.y < 0.0f || texCoord.y > 1.0f)
		 return shadowColor;
	 
	 if (texCoord.z < 0.0f || texCoord.z > 1.0f)
		 return shadowColor;

	//texCoord.x = (1.0 / MAX_CASCADE_NUM)*(index + texCoord.x);

	// ((index*1024) + (texCoord.x*1024)) / (1024*4)
	// ((index*1024) / (1024*4)) + ((texCoord.x*1024) / (1024*4))
	// (index / 4) + (texcoord.x / 4)
	texCoord.x = ((float)index + texCoord.x) / (float)MAX_CASCADE_NUM;


	texCoord.z -= Bias;
	
	float	litSum = 0;
	float	shadowPixel;
	
	for (float y=-1.5; y<=1.5; y += 1.0)
	{
		for (float x=-1.5; x<=1.5; x += 1.0)
		{
//			float4	texDepth = texShadowMap.Sample(samplerBorder,texCoord.xy + float2(x/1024.0f,y/1024.0f));
//			litSum += (PosShadowSpace.z - texDepth.r  <= 0.0f );		
			
			litSum += texShadowMap.SampleCmpLevelZero(samplerComp,texCoord.xy + float2(x/(1024.0f*MAX_CASCADE_NUM),y/1024.0f),texCoord.z );
		}
	}
	shadowColor = litSum / 16.0f;
	
	//shadowColor = shadowColor * 0.7f + 0.3f;
	shadowColor = shadowColor * 0.5f + 0.5f;
	
	if (texCoord.z > 1)
	{
		shadowColor = 1;
	}
	if (texCoord.z < 0)
	{
		shadowColor = 1;
	}
	return shadowColor;
}


float3 CalcNormalWithTri(float3 p0,float3 p1,float3 p2)
{
	float3 n;
	float3 r,u,v;
	
	u = p1 - p0;
	v = p2 - p0;

	r = cross(u,v);
	n = normalize(r);

	return n;
}
void matrix_from_Quaternion(out matrix mat, float4 q)
{
    float xx = q.x*q.x;
	float yy = q.y*q.y;
	float zz = q.z*q.z;
	
	float xy = q.x*q.y; 
	float xz = q.x*q.z;
	float yz = q.y*q.z;

	float wx = q.w*q.x;
	float wy = q.w*q.y;
	float wz = q.w*q.z;

    
    mat._11 = 1 - 2 * ( yy + zz ); 
    mat._12 =     2 * ( xy - wz );
    mat._13 =     2 * ( xz + wy );

    mat._21 =     2 * ( xy + wz );
    mat._22 = 1 - 2 * ( xx + zz );
    mat._23 =     2 * ( yz - wx );

    mat._31 =     2 * ( xz - wy );
    mat._32 =     2 * ( yz + wx );
    mat._33 = 1 - 2 * ( xx + yy );

    mat._14 = mat._24 = mat._34 = 0.0f;
    mat._41 = mat._42 = mat._43 = 0.0f;
    mat._44 = 1.0f;

}