//=============================================================================
//
// モデルの処理 [model.cpp]
// Author : (Supports millions of polygons)
//
//=============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include "main.h"
#include "model.h"
#include "camera.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define VALUE_MOVE_MODEL    (0.50f)                // 移動速度
#define RATE_MOVE_MODEL     (0.20f)                // 移動慣性係数
#define VALUE_ROTATE_MODEL  (XM_PI * 0.05f)        // 回転速度
#define RATE_ROTATE_MODEL   (0.20f)                // 回転慣性係数
#define SCALE_MODEL         (10.0f)                // 回転慣性係数

//*****************************************************************************
// 構造体定義
//*****************************************************************************

// マテリアル構造体
struct MODEL_MATERIAL
{
    char Name[256];
    MATERIAL Material;
    char TextureName[256];
};

// 描画サブセット構造体
struct SUBSET
{
    unsigned int StartIndex;
    unsigned int IndexNum;
    MODEL_MATERIAL Material;
};

// モデル構造体
struct MODEL
{
    VERTEX_3D* VertexArray;
    unsigned int VertexNum;
    unsigned int* IndexArray;
    unsigned int IndexNum;
    SUBSET* SubsetArray;
    unsigned int SubsetNum;
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void LoadObj(char* FileName, MODEL* Model);
void LoadMaterial(char* FileName, MODEL_MATERIAL** MaterialArray, unsigned int* MaterialNum);

//=============================================================================
// 初期化処理
//=============================================================================
void LoadModel(char* FileName, DX11_MODEL* Model)
{
    MODEL model;

    LoadObj(FileName, &model);

    // 頂点バッファ生成
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(VERTEX_3D) * model.VertexNum;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = model.VertexArray;

        GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
    }

    // インデックスバッファ生成
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(unsigned int) * model.IndexNum;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = model.IndexArray;

        GetDevice()->CreateBuffer(&bd, &sd, &Model->IndexBuffer);
    }

    // サブセット設定
    {
        Model->SubsetArray = new DX11_SUBSET[model.SubsetNum];
        Model->SubsetNum = model.SubsetNum;

        for (unsigned int i = 0; i < model.SubsetNum; i++)
        {
            Model->SubsetArray[i].StartIndex = model.SubsetArray[i].StartIndex;
            Model->SubsetArray[i].IndexNum = model.SubsetArray[i].IndexNum;

            Model->SubsetArray[i].Material.Material = model.SubsetArray[i].Material.Material;

            Model->SubsetArray[i].Material.Texture = NULL;
            D3DX11CreateShaderResourceViewFromFile(
                GetDevice(),
                model.SubsetArray[i].Material.TextureName,
                NULL, NULL,
                &Model->SubsetArray[i].Material.Texture,
                NULL);
        }
    }

    delete[] model.VertexArray;
    delete[] model.IndexArray;
    delete[] model.SubsetArray;
}

//=============================================================================
// 終了処理
//=============================================================================
void UnloadModel(DX11_MODEL* Model)
{
    for (unsigned int i = 0; i < Model->SubsetNum; i++)
    {
        if (Model->SubsetArray[i].Material.Texture)
        {
            Model->SubsetArray[i].Material.Texture->Release();
            Model->SubsetArray[i].Material.Texture = NULL;
        }
    }

    if (Model->VertexBuffer) Model->VertexBuffer->Release();
    if (Model->IndexBuffer)  Model->IndexBuffer->Release();
    if (Model->SubsetArray)  delete[] Model->SubsetArray;
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawModel(DX11_MODEL* Model)
{
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &Model->VertexBuffer, &stride, &offset);

    // --- Use DXGI_FORMAT_R32_UINT here ---
    GetDeviceContext()->IASetIndexBuffer(Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (unsigned int i = 0; i < Model->SubsetNum; i++)
    {
        SetMaterial(Model->SubsetArray[i].Material.Material);

        if (Model->SubsetArray[i].Material.Material.noTexSampling == 0)
        {
            GetDeviceContext()->PSSetShaderResources(0, 1, &Model->SubsetArray[i].Material.Texture);
        }

        GetDeviceContext()->DrawIndexed(Model->SubsetArray[i].IndexNum, Model->SubsetArray[i].StartIndex, 0);
    }
}

//=============================================================================
// モデル読込
//=============================================================================
void LoadObj(char* FileName, MODEL* Model)
{
    FILE* file = fopen(FileName, "rt");
    if (file == NULL)
    {
        printf("エラー:LoadModel %s \n", FileName);
        return;
    }

    // Use unsigned int for counters
    unsigned int positionNum = 0, normalNum = 0, texcoordNum = 0;
    unsigned int vertexNum = 0, indexNum = 0, subsetNum = 0, in = 0;

    MODEL_MATERIAL* materialArray = NULL;
    unsigned int materialNum = 0;

    char str[256], * s, c;

    //要素数カウント
    while (true)
    {
        if (fscanf(file, "%s", str) != 1) break;
        if (feof(file)) break;

        if (strcmp(str, "v") == 0) positionNum++;
        else if (strcmp(str, "vn") == 0) normalNum++;
        else if (strcmp(str, "vt") == 0) texcoordNum++;
        else if (strcmp(str, "usemtl") == 0) subsetNum++;
        else if (strcmp(str, "f") == 0)
        {
            in = 0;
            do {
                if (fscanf(file, "%s", str) != 1) break;
                vertexNum++; in++;
                c = fgetc(file);
            } while (c != '\n' && c != '\r');
            if (in == 4) in = 6;
            indexNum += in;
        }
    }

    XMFLOAT3* positionArray = new XMFLOAT3[positionNum];
    XMFLOAT3* normalArray = new XMFLOAT3[normalNum];
    XMFLOAT2* texcoordArray = new XMFLOAT2[texcoordNum];

    Model->VertexArray = new VERTEX_3D[vertexNum];
    Model->VertexNum = vertexNum;
    Model->IndexArray = new unsigned int[indexNum];
    Model->IndexNum = indexNum;
    Model->SubsetArray = new SUBSET[subsetNum];
    Model->SubsetNum = subsetNum;

    XMFLOAT3* position = positionArray;
    XMFLOAT3* normal = normalArray;
    XMFLOAT2* texcoord = texcoordArray;
    unsigned int vc = 0, ic = 0, sc = 0;

    fseek(file, 0, SEEK_SET);

    while (true)
    {
        if (fscanf(file, "%s", str) != 1) break;
        if (feof(file)) break;

        if (strcmp(str, "mtllib") == 0)
        {
            fscanf(file, "%s", str);
            char path[256];
            strcpy(path, FileName);
            char* adr = path;
            char* ans = adr;
            while (1)
            {
                adr = strstr(adr, "/");
                if (adr == NULL) break;
                else ans = adr;
                adr++;
            }
            if (path != ans) ans++;
            *ans = 0;
            strcat(path, str);
            LoadMaterial(path, &materialArray, &materialNum);
        }
        else if (strcmp(str, "o") == 0)
        {
            fscanf(file, "%s", str);
        }
        else if (strcmp(str, "v") == 0)
        {
            fscanf(file, "%f %f %f", &position->x, &position->y, &position->z);
            position->x *= SCALE_MODEL;
            position->y *= SCALE_MODEL;
            position->z *= SCALE_MODEL;
            position++;
        }
        else if (strcmp(str, "vn") == 0)
        {
            fscanf(file, "%f %f %f", &normal->x, &normal->y, &normal->z);
            normal++;
        }
        else if (strcmp(str, "vt") == 0)
        {
            fscanf(file, "%f %f", &texcoord->x, &texcoord->y);
            texcoord->y = 1.0f - texcoord->y;
            texcoord++;
        }
        else if (strcmp(str, "usemtl") == 0)
        {
            fscanf(file, "%s", str);
            if (sc != 0)
                Model->SubsetArray[sc - 1].IndexNum = ic - Model->SubsetArray[sc - 1].StartIndex;
            Model->SubsetArray[sc].StartIndex = ic;

            for (unsigned int i = 0; i < materialNum; i++)
            {
                if (strcmp(str, materialArray[i].Name) == 0)
                {
                    Model->SubsetArray[sc].Material.Material = materialArray[i].Material;
                    strcpy(Model->SubsetArray[sc].Material.TextureName, materialArray[i].TextureName);
                    strcpy(Model->SubsetArray[sc].Material.Name, materialArray[i].Name);
                    break;
                }
            }
            sc++;
        }
        else if (strcmp(str, "f") == 0)
        {
            in = 0;
            do {
                if (fscanf(file, "%s", str) != 1) break;
                s = strtok(str, "/");
                Model->VertexArray[vc].Position = positionArray[atoi(s) - 1];
                if (s[strlen(s) + 1] != '/')
                {
                    s = strtok(NULL, "/");
                    Model->VertexArray[vc].TexCoord = texcoordArray[atoi(s) - 1];
                }
                s = strtok(NULL, "/");
                Model->VertexArray[vc].Normal = normalArray[atoi(s) - 1];
                Model->VertexArray[vc].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
                Model->IndexArray[ic] = vc;
                ic++; vc++; in++;
                c = fgetc(file);
            } while (c != '\n' && c != '\r');
            if (in == 4)
            {
                Model->IndexArray[ic] = vc - 4; ic++;
                Model->IndexArray[ic] = vc - 2; ic++;
            }
        }
    }

    if (sc != 0)
        Model->SubsetArray[sc - 1].IndexNum = ic - Model->SubsetArray[sc - 1].StartIndex;

    delete[] positionArray;
    delete[] normalArray;
    delete[] texcoordArray;
    delete[] materialArray;
    fclose(file);
}

//=============================================================================
// マテリアル読み込み
//=============================================================================
void LoadMaterial(char* FileName, MODEL_MATERIAL** MaterialArray, unsigned int* MaterialNum)
{
    char str[256];
    FILE* file = fopen(FileName, "rt");
    if (file == NULL)
    {
        printf("エラー:LoadMaterial %s \n", FileName);
        return;
    }

    MODEL_MATERIAL* materialArray;
    unsigned int materialNum = 0;

    //要素数カウント
    while (true)
    {
        if (fscanf(file, "%s", str) != 1) break;
        if (feof(file)) break;
        if (strcmp(str, "newmtl") == 0) materialNum++;
    }

    materialArray = new MODEL_MATERIAL[materialNum];
    ZeroMemory(materialArray, sizeof(MODEL_MATERIAL) * materialNum);

    int mc = -1;
    fseek(file, 0, SEEK_SET);

    while (true)
    {
        if (fscanf(file, "%s", str) != 1) break;
        if (feof(file)) break;
        if (strcmp(str, "newmtl") == 0)
        {
            mc++;
            fscanf(file, "%s", materialArray[mc].Name);
            strcpy(materialArray[mc].TextureName, "");
            materialArray[mc].Material.noTexSampling = 1;
        }
        else if (strcmp(str, "Ka") == 0)
        {
            fscanf(file, "%f %f %f", &materialArray[mc].Material.Ambient.x, &materialArray[mc].Material.Ambient.y, &materialArray[mc].Material.Ambient.z);
            materialArray[mc].Material.Ambient.w = 1.0f;
        }
        else if (strcmp(str, "Kd") == 0)
        {
            fscanf(file, "%f %f %f", &materialArray[mc].Material.Diffuse.x, &materialArray[mc].Material.Diffuse.y, &materialArray[mc].Material.Diffuse.z);
            if ((materialArray[mc].Material.Diffuse.x + materialArray[mc].Material.Diffuse.y + materialArray[mc].Material.Diffuse.z) == 0.0f)
            {
                materialArray[mc].Material.Diffuse.x = materialArray[mc].Material.Diffuse.y = materialArray[mc].Material.Diffuse.z = 1.0f;
            }
            materialArray[mc].Material.Diffuse.w = 1.0f;
        }
        else if (strcmp(str, "Ks") == 0)
        {
            fscanf(file, "%f %f %f", &materialArray[mc].Material.Specular.x, &materialArray[mc].Material.Specular.y, &materialArray[mc].Material.Specular.z);
            materialArray[mc].Material.Specular.w = 1.0f;
        }
        else if (strcmp(str, "Ns") == 0)
        {
            fscanf(file, "%f", &materialArray[mc].Material.Shininess);
        }
        else if (strcmp(str, "d") == 0)
        {
            fscanf(file, "%f", &materialArray[mc].Material.Diffuse.w);
        }
        else if (strcmp(str, "map_Kd") == 0)
        {
            fscanf(file, "%s", str);
            char path[256];
            strcpy(path, FileName);
            char* adr = path;
            char* ans = adr;
            while (1)
            {
                adr = strstr(adr, "/");
                if (adr == NULL) break;
                else ans = adr;
                adr++;
            }
            if (path != ans) ans++;
            *ans = 0;
            strcat(path, str);
            strcat(materialArray[mc].TextureName, path);
            materialArray[mc].Material.noTexSampling = 0;
        }
    }

    *MaterialArray = materialArray;
    *MaterialNum = materialNum;
    fclose(file);
}

// モデルの全マテリアルのディフューズを取得する。Max16個分にしてある
void GetModelDiffuse(DX11_MODEL* Model, XMFLOAT4* diffuse)
{
    int max = (Model->SubsetNum < MODEL_MAX_MATERIAL) ? Model->SubsetNum : MODEL_MAX_MATERIAL;
    for (unsigned int i = 0; i < max; i++)
    {
        diffuse[i] = Model->SubsetArray[i].Material.Material.Diffuse;
    }
}

// モデルの指定マテリアルのディフューズをセットする。
void SetModelDiffuse(DX11_MODEL* Model, int mno, XMFLOAT4 diffuse)
{
    Model->SubsetArray[mno].Material.Material.Diffuse = diffuse;
}
