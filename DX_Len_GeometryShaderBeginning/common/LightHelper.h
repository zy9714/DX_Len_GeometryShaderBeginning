#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <cstring>
#include <DirectXMath.h>


// 环境光
struct directionalLight_struct
{
	directionalLight_struct() { memset(this, 0, sizeof(directionalLight_struct)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT3 Direction;
	float Pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 点光
struct pointLight_struct
{
	pointLight_struct() { memset(this, 0, sizeof(pointLight_struct)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	// 打包成4D向量: (Position, Range)
	DirectX::XMFLOAT3 Position;
	float Range;

	// 打包成4D向量: (A0, A1, A2, Pad)
	DirectX::XMFLOAT3 Att;
	float Pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 聚光灯
struct spotLight_struct
{
	spotLight_struct() { memset(this, 0, sizeof(spotLight_struct)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	// 打包成4D向量: (Position, Range)
	DirectX::XMFLOAT3 Position;
	float Range;

	// 打包成4D向量: (Direction, Spot)
	DirectX::XMFLOAT3 Direction;
	float Spot;

	// 打包成4D向量: (Att, Pad)
	DirectX::XMFLOAT3 Att;
	float Pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 物体表面材质
struct material_struct
{
	material_struct() { memset(this, 0, sizeof(material_struct)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; // w = 镜面反射强度
	DirectX::XMFLOAT4 Reflect;
};

#endif