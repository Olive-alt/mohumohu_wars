#include <vector>
#include <d3dcompiler.h>
#include "main.h"
#include "renderer.h"
#include "debugline.h"
struct DebugLineVertex {
    XMFLOAT3 pos;
    XMFLOAT4 color;
};

static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_context = nullptr;
static ID3D11Buffer* g_vertexBuffer = nullptr;
static ID3D11InputLayout* g_inputLayout = nullptr;
static ID3D11VertexShader* g_vs = nullptr;
static ID3D11PixelShader* g_ps = nullptr;
static int g_maxLines = 1024;

static std::vector<DebugLineVertex> g_lines;

static const char* g_vs_code =
"cbuffer cb : register(b0) { matrix vp; };"
"struct VS_IN { float3 pos : POSITION; float4 col : COLOR0; };"
"struct VS_OUT { float4 pos : SV_Position; float4 col : COLOR0; };"
"VS_OUT main(VS_IN vin) {"
"    VS_OUT vout;"
"    vout.pos = mul(float4(vin.pos,1), vp);"
"    vout.col = vin.col;"
"    return vout;"
"}";

static const char* g_ps_code =
"struct PS_IN { float4 pos : SV_Position; float4 col : COLOR0; };"
"float4 main(PS_IN pin) : SV_Target { return pin.col; }";

static HRESULT CompileShader(const char* code, const char* entry, const char* target, ID3DBlob** blob)
{
    ID3DBlob* error = nullptr;
    HRESULT hr = D3DCompile(code, strlen(code), nullptr, nullptr, nullptr, entry, target, 0, 0, blob, &error);
    if (error) error->Release();
    return hr;
}

void DebugLine_Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int maxLines)
{
    g_device = device;
    g_context = context;
    g_maxLines = maxLines;

    // Dynamic vertex buffer
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(DebugLineVertex) * 2 * maxLines;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&desc, nullptr, &g_vertexBuffer);

    // Compile shaders
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    CompileShader(g_vs_code, "main", "vs_4_0", &vsBlob);
    CompileShader(g_ps_code, "main", "ps_4_0", &psBlob);
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps);

    // Input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_inputLayout);

    vsBlob->Release();
    psBlob->Release();
}

void DebugLine_BeginFrame()
{
    g_lines.clear();
}

void DebugLine_DrawLine(XMFLOAT3 start, XMFLOAT3 end, XMFLOAT4 color)
{
    if ((int)g_lines.size() / 2 < g_maxLines) {
        g_lines.push_back({ start, color });
        g_lines.push_back({ end,   color });
    }
}

struct CB { XMMATRIX vp; };

void DebugLine_Render(const XMMATRIX& viewProj)
{
    if (g_lines.empty()) return;

    // --- Save previous pipeline state ---
    ID3D11InputLayout* prevInputLayout = nullptr;
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    ID3D11Buffer* prevVB = nullptr;
    UINT prevStride = 0, prevOffset = 0;
    D3D11_PRIMITIVE_TOPOLOGY prevTopology;
    ID3D11Buffer* prevCB = nullptr;

    g_context->IAGetInputLayout(&prevInputLayout);
    g_context->VSGetShader(&prevVS, nullptr, 0);
    g_context->PSGetShader(&prevPS, nullptr, 0);
    g_context->IAGetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);
    prevTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    g_context->IAGetPrimitiveTopology(&prevTopology);
    g_context->VSGetConstantBuffers(0, 1, &prevCB);

    // --- Update vertex buffer ---
    D3D11_MAPPED_SUBRESOURCE mapped;
    g_context->Map(g_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, g_lines.data(), g_lines.size() * sizeof(DebugLineVertex));
    g_context->Unmap(g_vertexBuffer, 0);

    // --- Set debug line pipeline ---
    g_context->IASetInputLayout(g_inputLayout);
    g_context->VSSetShader(g_vs, nullptr, 0);
    g_context->PSSetShader(g_ps, nullptr, 0);

    // Constant buffer for viewProj
    ID3D11Buffer* cb = nullptr;
    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(XMMATRIX);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    g_device->CreateBuffer(&cbd, nullptr, &cb);

    D3D11_MAPPED_SUBRESOURCE cbMapped;
    g_context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &cbMapped);
    *(XMMATRIX*)cbMapped.pData = XMMatrixTranspose(viewProj);
    g_context->Unmap(cb, 0);
    g_context->VSSetConstantBuffers(0, 1, &cb);

    UINT stride = sizeof(DebugLineVertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    g_context->Draw((UINT)g_lines.size(), 0);

    cb->Release();

    // --- Restore previous pipeline state ---
    g_context->IASetInputLayout(prevInputLayout);
    g_context->VSSetShader(prevVS, nullptr, 0);
    g_context->PSSetShader(prevPS, nullptr, 0);
    g_context->IASetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);
    g_context->IASetPrimitiveTopology(prevTopology);
    g_context->VSSetConstantBuffers(0, 1, &prevCB);

    if (prevInputLayout) prevInputLayout->Release();
    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
    if (prevVB) prevVB->Release();
    if (prevCB) prevCB->Release();
}

void DebugLine_Shutdown()
{
    if (g_vertexBuffer) { g_vertexBuffer->Release(); g_vertexBuffer = nullptr; }
    if (g_inputLayout) { g_inputLayout->Release();  g_inputLayout = nullptr; }
    if (g_vs) { g_vs->Release();           g_vs = nullptr; }
    if (g_ps) { g_ps->Release();           g_ps = nullptr; }
    g_lines.clear();
}

// ãÖÇÃÉèÉCÉÑÅ[ÉtÉåÅ[ÉÄÇï`âÊÇ∑ÇÈä÷êî
void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices)
{
    // XYïΩñ ÇÃâ~Çï`âÊ
    for (int i = 0; i < slices; ++i) {
        float theta1 = XM_2PI * i / slices;
        float theta2 = XM_2PI * (i + 1) / slices;
        XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y + sinf(theta1) * radius, center.z);
        XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y + sinf(theta2) * radius, center.z);
        DebugLine_DrawLine(p1, p2, color);
    }

    // XZïΩñ ÇÃâ~Çï`âÊ
    for (int i = 0; i < slices; ++i) {
        float theta1 = XM_2PI * i / slices;
        float theta2 = XM_2PI * (i + 1) / slices;
        XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y, center.z + sinf(theta1) * radius);
        XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y, center.z + sinf(theta2) * radius);
        DebugLine_DrawLine(p1, p2, color);
    }

    // YZïΩñ ÇÃâ~Çï`âÊ
    for (int i = 0; i < slices; ++i) {
        float theta1 = XM_2PI * i / slices;
        float theta2 = XM_2PI * (i + 1) / slices;
        XMFLOAT3 p1(center.x, center.y + cosf(theta1) * radius, center.z + sinf(theta1) * radius);
        XMFLOAT3 p2(center.x, center.y + cosf(theta2) * radius, center.z + sinf(theta2) * radius);
        DebugLine_DrawLine(p1, p2, color);
    }
}

// Draw a capsule's wireframe outline (clean style)
void DrawDebugCapsuleOutline(
    const XMFLOAT3& a, // base
    const XMFLOAT3& b, // tip
    float radius,
    const XMFLOAT4& color,
    int slices = 20,   // number of rings/segments in each circle
    int hemiRings = 4  // number of latitude rings on each cap (excluding the pole)
)
{
    using namespace DirectX;

    // Axis (capsule main direction)
    XMVECTOR va = XMLoadFloat3(&a);
    XMVECTOR vb = XMLoadFloat3(&b);
    XMVECTOR axis = XMVector3Normalize(vb - va);
    float length = XMVectorGetX(XMVector3Length(vb - va));

    // Orthonormal basis
    XMVECTOR ref = fabs(XMVectorGetX(XMVector3Dot(axis, XMVectorSet(1, 0, 0, 0)))) > 0.99f
        ? XMVectorSet(0, 1, 0, 0) : XMVectorSet(1, 0, 0, 0);
    XMVECTOR n1 = XMVector3Normalize(XMVector3Cross(axis, ref));
    XMVECTOR n2 = XMVector3Normalize(XMVector3Cross(axis, n1));

    // Draw rings at both ends (the equators)
    for (int i = 0; i < slices; ++i) {
        float t1 = XM_2PI * i / slices;
        float t2 = XM_2PI * (i + 1) / slices;
        XMVECTOR off1 = XMVectorScale(n1, cosf(t1) * radius) + XMVectorScale(n2, sinf(t1) * radius);
        XMVECTOR off2 = XMVectorScale(n1, cosf(t2) * radius) + XMVectorScale(n2, sinf(t2) * radius);

        // Base ring
        XMFLOAT3 pA1, pA2;
        XMStoreFloat3(&pA1, va + off1);
        XMStoreFloat3(&pA2, va + off2);
        DebugLine_DrawLine(pA1, pA2, color);

        // Top ring
        XMFLOAT3 pB1, pB2;
        XMStoreFloat3(&pB1, vb + off1);
        XMStoreFloat3(&pB2, vb + off2);
        DebugLine_DrawLine(pB1, pB2, color);

        // Connect side lines (cylinder)
        DebugLine_DrawLine(pA1, pB1, color);
    }

    // Draw latitude rings (hemiRings) for each hemisphere (caps)
    for (int h = 1; h < hemiRings; ++h) {
        float phi = (XM_PIDIV2 * h) / hemiRings; // 0 (equator) to pi/2 (pole)
        float y = sinf(phi) * radius;
        float r = cosf(phi) * radius;

        // Lower hemisphere (base cap)
        XMVECTOR capCenterA = va;
        XMVECTOR capDirA = -axis;
        for (int i = 0; i < slices; ++i) {
            float t1 = XM_2PI * i / slices;
            float t2 = XM_2PI * (i + 1) / slices;
            XMVECTOR off1 = XMVectorScale(n1, cosf(t1) * r) + XMVectorScale(n2, sinf(t1) * r);
            XMVECTOR off2 = XMVectorScale(n1, cosf(t2) * r) + XMVectorScale(n2, sinf(t2) * r);
            XMVECTOR ring1 = capCenterA + off1 + XMVectorScale(capDirA, y);
            XMVECTOR ring2 = capCenterA + off2 + XMVectorScale(capDirA, y);
            XMFLOAT3 fp1, fp2;
            XMStoreFloat3(&fp1, ring1);
            XMStoreFloat3(&fp2, ring2);
            DebugLine_DrawLine(fp1, fp2, color);
        }

        // Upper hemisphere (tip cap)
        XMVECTOR capCenterB = vb;
        XMVECTOR capDirB = axis;
        for (int i = 0; i < slices; ++i) {
            float t1 = XM_2PI * i / slices;
            float t2 = XM_2PI * (i + 1) / slices;
            XMVECTOR off1 = XMVectorScale(n1, cosf(t1) * r) + XMVectorScale(n2, sinf(t1) * r);
            XMVECTOR off2 = XMVectorScale(n1, cosf(t2) * r) + XMVectorScale(n2, sinf(t2) * r);
            XMVECTOR ring1 = capCenterB + off1 + XMVectorScale(capDirB, y);
            XMVECTOR ring2 = capCenterB + off2 + XMVectorScale(capDirB, y);
            XMFLOAT3 fp1, fp2;
            XMStoreFloat3(&fp1, ring1);
            XMStoreFloat3(&fp2, ring2);
            DebugLine_DrawLine(fp1, fp2, color);
        }
    }
}
