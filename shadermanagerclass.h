////////////////////////////////////////////////////////////////////////////////
// Filename: shadermanagerclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SHADERMANAGERCLASS_H_
#define _SHADERMANAGERCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
//#include "textureshaderclass.h"
//#include "lightshaderclass.h"
#include "normalmapshaderclass.h"
//#include "specmapshaderclass.h"
//#include "fogshaderclass.h"
//#include "fontshaderclass.h"
//#include "clipplaneshaderclass.h"
//#include "translateshaderclass.h"
//#include "transparentshaderclass.h"
//#include "reflectionshaderclass.h"
////////////////////////////////////////////////////////////////////////////////
// Class name: ShaderManagerClass
////////////////////////////////////////////////////////////////////////////////
class ShaderManagerClass
{
public:
	ShaderManagerClass();
	ShaderManagerClass(const ShaderManagerClass&);
	~ShaderManagerClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();

	bool RenderTextureShader(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*);
	bool RenderLightShader(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);
	bool RenderNormalMapShader(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);
	bool RenderFogShader(
		ID3D11DeviceContext* ctx,
		int indexCount,
		DirectX::XMMATRIX world,
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX proj,
		ID3D11ShaderResourceView* tex,
		float fogStart,
		float fogEnd
	);

	bool RenderTranslateShader(
		ID3D11DeviceContext* deviceContext,
		int indexCount,
		XMMATRIX world,
		XMMATRIX view,
		XMMATRIX proj,
		ID3D11ShaderResourceView* texture,
		float translation
	);

	bool RenderClipPlaneShader(
		ID3D11DeviceContext* deviceContext,
		int indexCount,
		XMMATRIX worldMatrix,
		XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView* texture,
		XMFLOAT4 clipPlane
	);

	bool RenderFontShader(
		ID3D11DeviceContext* deviceContext,
		int indexCount,
		XMMATRIX worldMatrix,
		XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView* texture,
		XMFLOAT4 pixelColor
	);

	    /// Dispatch a Transparency pass
	bool RenderTransparentShader(
		ID3D11DeviceContext * deviceContext,
		int indexCount,
		XMMATRIX worldMatrix,
		XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView * texture,
		float blendAmount
	 );

	bool RenderReflectionShader(
		ID3D11DeviceContext * deviceContext,
		int indexCount,
		XMMATRIX worldMatrix,
		XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView * texture,
		ID3D11ShaderResourceView * reflectionTexture,
		XMMATRIX reflectionMatrix
		 );

	bool RenderSpecMapShader(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*,
		XMFLOAT3, XMFLOAT4, XMFLOAT3, XMFLOAT4, float);

	//FontShaderClass* GetFontShader() const { return m_FontShader; }
private:
	//TextureShaderClass* m_TextureShader;
	//LightShaderClass* m_LightShader;
	NormalMapShaderClass* m_NormalMapShader;
	//SpecMapShaderClass* m_SpecMapShader;
	//FogShaderClass* m_FogShader;
	//FontShaderClass* m_FontShader;
	//ClipPlaneShaderClass* m_ClipPlaneShader;
	//TranslateShaderClass* m_TranslateShader;
	//TransparentShaderClass* m_TransparentShader;
	//ReflectionShaderClass* m_ReflectionShader;
};

#endif