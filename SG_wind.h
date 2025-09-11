#pragma once
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
class WIND
{
private:
	BOOL use;
	XMFLOAT3 movePower;
	int countTime;
	int spawnTime;
	int spawnCount;

public:
	HRESULT InitSGwind(void);
	void UninitSGwind(void);
	void UpdateSGwind(void);

	void SetSGwind(void);
	void FinishSGwind(void);
};
