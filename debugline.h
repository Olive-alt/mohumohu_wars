#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

// Call these at the correct times:
void DebugLine_Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int maxLines = 1024);
void DebugLine_BeginFrame();
void DebugLine_DrawLine(XMFLOAT3 start, XMFLOAT3 end, XMFLOAT4 color); // call this to enqueue a line
void DebugLine_Render(const XMMATRIX& viewProj);
void DebugLine_Shutdown();

void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices = 20);

void DrawDebugCapsuleOutline(const XMFLOAT3& a, const XMFLOAT3& b, float radius, const XMFLOAT4& color, int slices, int hemiRings);
