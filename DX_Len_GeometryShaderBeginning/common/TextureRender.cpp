#include "TextureRender.h"
#include "DXTrace.h"
using namespace Microsoft::WRL;

TextureRender::TextureRender(ComPtr<ID3D11Device> device, int texWidth, int texHeight, bool generateMips)
	: mGenerateMips(generateMips)
{
	// ******************
	// 1. 创建纹理
	//

	ComPtr<ID3D11Texture2D> texture;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = (mGenerateMips ? 0 : 1);	// 0为完整mipmap链
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// 现在texture用于新建纹理
	HR(device->CreateTexture2D(&texDesc, nullptr, texture.ReleaseAndGetAddressOf()));

	// ******************
	// 2. 创建纹理对应的着色器资源视图
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;	// -1使用所有的mip等级
	HR(device->CreateShaderResourceView(texture.Get(), &srvDesc, mOutputTextureRT_SRV.GetAddressOf()));
	 
	// ******************
	// 3. 创建纹理对应的渲染目标视图
	//	

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	HR(device->CreateRenderTargetView(texture.Get(), &rtvDesc, mOutputTextureRTV.GetAddressOf()));

	// ******************
	// 4. 创建与纹理等宽高的深度/模板缓冲区和对应的视图
	//

	ComPtr<ID3D11Texture2D> dsTex;
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	device->CreateTexture2D(&texDesc, nullptr, dsTex.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(device->CreateDepthStencilView(dsTex.Get(), &dsvDesc, mOutputTextureDSV.GetAddressOf()));

	// ******************
	// 5. 初始化视口
	//
	mOutputViewPort.TopLeftX = 0.0f;
	mOutputViewPort.TopLeftY = 0.0f;
	mOutputViewPort.Width = static_cast<float>(texWidth);
	mOutputViewPort.Height = static_cast<float>(texHeight);
	mOutputViewPort.MinDepth = 0.0f;
	mOutputViewPort.MaxDepth = 1.0f;
}

TextureRender::~TextureRender()
{
}

void TextureRender::Begin(ComPtr<ID3D11DeviceContext> deviceContext)
{
	// 缓存渲染目标和深度模板视图
	deviceContext->OMGetRenderTargets(1, mCacheRTV.GetAddressOf(), mCacheDSV.GetAddressOf());
	// 缓存视口
	UINT numViewports = 1;
	deviceContext->RSGetViewports(&numViewports, &mCacheViewPort);


	// 清空缓冲区
	float wite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float black[4] = { 0.0f, 0.0f, 0.0f,1.0f };
	deviceContext->ClearRenderTargetView(mOutputTextureRTV.Get(), wite);
	deviceContext->ClearDepthStencilView(mOutputTextureDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// 设置渲染目标和深度模板视图
	deviceContext->OMSetRenderTargets(1, mOutputTextureRTV.GetAddressOf(), mOutputTextureDSV.Get());
	// 设置视口
	deviceContext->RSSetViewports(1, &mOutputViewPort);
}

void TextureRender::End(ComPtr<ID3D11DeviceContext> deviceContext)
{
	// 恢复默认设定
	deviceContext->RSSetViewports(1, &mCacheViewPort);
	deviceContext->OMSetRenderTargets(1, mCacheRTV.GetAddressOf(), mCacheDSV.Get());

	// 若之前有指定需要mipmap链，则生成
	if (mGenerateMips)
	{
		deviceContext->GenerateMips(mOutputTextureRT_SRV.Get());
	}
	
	// 清空临时缓存的渲染目标视图和深度模板视图
	mCacheDSV.Reset();
	mCacheRTV.Reset();
}

ComPtr<ID3D11ShaderResourceView> TextureRender::GetOutputTextureRT()
{
	return mOutputTextureRT_SRV;
}

ComPtr<ID3D11ShaderResourceView> TextureRender::GetOutputTextureDS()
{
	return mOutputTextureDS_SRV;
}