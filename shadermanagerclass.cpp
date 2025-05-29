////////////////////////////////////////////////////////////////////////////////
// Filename: shadermanagerclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "shadermanagerclass.h"


ShaderManagerClass::ShaderManagerClass()
{
	m_TextureShader = 0;
	m_LightShader = 0;
	m_NormalMapShader = 0;
	m_FogShader = 0;
	m_ClipPlaneShader = 0;
	m_TranslateShader = 0;
}



ShaderManagerClass::ShaderManagerClass(const ShaderManagerClass& other)
{
}


ShaderManagerClass::~ShaderManagerClass()
{
}


bool ShaderManagerClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	// Create and initialize the reflection shader object.
	m_ReflectionShader = new ReflectionShaderClass;
	result = m_ReflectionShader->Initialize(device, hwnd);
	if (!result)
	return false;

	    // Create and initialize the transparent shader object.
	m_TransparentShader = new TransparentShaderClass;
	result = m_TransparentShader->Initialize(device, hwnd);
	if (!result)
	return false;

	// Create and initialize the translate shader object.
	m_TranslateShader = new TranslateShaderClass;
	result = m_TranslateShader->Initialize(device, hwnd);
	if (!result)
	{
		return false;

	}

	// Create and initialize the ClipPlane shader object.
	m_ClipPlaneShader = new ClipPlaneShaderClass;

	result = m_ClipPlaneShader->Initialize(device, hwnd);
	if (!result)
	{
		return false;
	}

	// Create and initialize the font shader object.
	m_FontShader = new FontShaderClass;
	if (!m_FontShader->Initialize(device, hwnd))
		return false;

	// Create and initialize the texture shader object.
	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(device, hwnd);
	if(!result)
	{
		return false;
	}

	// Create and initialize the light shader object.
	m_LightShader = new LightShaderClass;

	result = m_LightShader->Initialize(device, hwnd);
	if(!result)
	{
		return false;
	}

	// Create and initialize the normal map shader object.
	m_NormalMapShader = new NormalMapShaderClass;

	result = m_NormalMapShader->Initialize(device, hwnd);
	if(!result)
	{
		return false;
	}

	// Create and initialize the normal map shader object.
	m_SpecMapShader = new SpecMapShaderClass;

	result = m_SpecMapShader->Initialize(device, hwnd);
	if (!result)
	{
		return false;
	}

	// Create and initialize the fog shader object.
	m_FogShader = new FogShaderClass;

	result = m_FogShader->Initialize(device, hwnd);
	if (!result)
	{
		return false;
	}

	return true;
}


void ShaderManagerClass::Shutdown()
{

	// Release the reflection shader object.
	if (m_ReflectionShader) {
	m_ReflectionShader->Shutdown();
	delete m_ReflectionShader;
	m_ReflectionShader = nullptr;
		
	}

	// Release the transparent shader object.
	if (m_TransparentShader) 
	{
		m_TransparentShader->Shutdown();
		delete m_TransparentShader;
		m_TransparentShader = nullptr;
		
	}

	// Release the translate shader object.
	if (m_TranslateShader) 
	{
		m_TranslateShader->Shutdown();
		delete m_TranslateShader;
		m_TranslateShader = nullptr;
		
	}
	if (m_ClipPlaneShader) {
		m_ClipPlaneShader->Shutdown();
		delete m_ClipPlaneShader;
		m_ClipPlaneShader = nullptr;
	}

	if (m_FontShader) {
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = nullptr;
	}

	// Release the normal map shader object.
	if(m_NormalMapShader)
	{
		m_NormalMapShader->Shutdown();
		delete m_NormalMapShader;
		m_NormalMapShader = 0;
	}

	// Release the light shader object.
	if(m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	// Release the texture shader object.
	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the Specular Map shader object.
	if (m_SpecMapShader)
	{
		m_SpecMapShader->Shutdown();
		delete m_SpecMapShader;
		m_SpecMapShader = 0;
	}

	//Release the fog shader object.
	if (m_FogShader)
	{
		m_FogShader->Shutdown();
		delete m_FogShader;
		m_FogShader = 0;
	}

	return;
}


bool ShaderManagerClass::RenderTextureShader(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
											 ID3D11ShaderResourceView* texture)
{
	bool result;


	result = m_TextureShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture);
	if(!result)
	{
		return false;
	}

	return true;
}


bool ShaderManagerClass::RenderLightShader(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
										   ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	bool result;


	result = m_LightShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, diffuseColor);
	if(!result)
	{
		return false;
	}

	return true;
}


bool ShaderManagerClass::RenderNormalMapShader(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
											   ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalTexture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	bool result;


	result = m_NormalMapShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, colorTexture, normalTexture, lightDirection, diffuseColor);
	if(!result)
	{
		return false;
	}

	return true;
}


bool ShaderManagerClass::RenderSpecMapShader(ID3D11DeviceContext* deviceContext,int indexCount,XMMATRIX worldMatrix,XMMATRIX viewMatrix,XMMATRIX projectionMatrix,ID3D11ShaderResourceView* texture1,ID3D11ShaderResourceView* texture2,ID3D11ShaderResourceView* texture3,
	XMFLOAT3 lightDirection,XMFLOAT4 diffuseColor,XMFLOAT3 cameraPosition,XMFLOAT4 specularColor,float specularPower)
{
	bool result;

	// © include indexCount here, before worldMatrix
	result = m_SpecMapShader->Render(deviceContext,indexCount,worldMatrix,viewMatrix,projectionMatrix,texture1,texture2,texture3,lightDirection,diffuseColor,cameraPosition,specularColor,specularPower);
	if (!result)
	{
		return false;
	}

	return true;
}


bool ShaderManagerClass::RenderFogShader(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float fogStart, float fogEnd)
{
	bool result;

	result = m_FogShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, fogStart, fogEnd);
	if (!result)
	{
		return false;
	}



	return true;
}

bool ShaderManagerClass::RenderFontShader(
	ID3D11DeviceContext* deviceContext,
	int indexCount,
	XMMATRIX worldMatrix,
	XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* texture,
	XMFLOAT4 pixelColor
) 
{
	return m_FontShader->Render(
		deviceContext,
		indexCount,
		worldMatrix,
		viewMatrix,
		projectionMatrix,
		texture,
		pixelColor
	);
}

bool ShaderManagerClass::RenderClipPlaneShader(
	ID3D11DeviceContext* deviceContext,
	int indexCount,
	XMMATRIX worldMatrix,
	XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* texture,
	XMFLOAT4 clipPlane
)
{
	return m_ClipPlaneShader
		? m_ClipPlaneShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, clipPlane)
		: false;
}

bool ShaderManagerClass::RenderTranslateShader(
	ID3D11DeviceContext* deviceContext,
	int indexCount,
	XMMATRIX world,
	XMMATRIX view,
	XMMATRIX proj,
	ID3D11ShaderResourceView* texture,
	float translation
) {
	return m_TranslateShader
		? m_TranslateShader->Render(deviceContext, indexCount, world, view, proj, texture, translation)
		: false;
}

bool ShaderManagerClass::RenderTransparentShader(
	ID3D11DeviceContext * deviceContext,
	int indexCount,
	XMMATRIX worldMatrix,
	XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView * texture,
	float blendAmount
	 ) {
	if (!m_TransparentShader) return false;
	return m_TransparentShader->Render(
		deviceContext,
		indexCount,
		worldMatrix,
		viewMatrix,
		projectionMatrix,
		texture,
		blendAmount
);
	
}

bool ShaderManagerClass::RenderReflectionShader(
ID3D11DeviceContext * deviceContext,
int indexCount,
XMMATRIX worldMatrix,
XMMATRIX viewMatrix,
XMMATRIX projectionMatrix,
ID3D11ShaderResourceView * texture,
ID3D11ShaderResourceView * reflectionTexture,
XMMATRIX reflectionMatrix
 ) {
return m_ReflectionShader->Render(
		deviceContext,
		indexCount,
		worldMatrix,
		viewMatrix,
		projectionMatrix,
		texture,
		reflectionTexture,
		reflectionMatrix
		 );
	
}