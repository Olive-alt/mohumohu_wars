#pragma once
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
class WIND
{
private:
	BOOL use;
	XMFLOAT3 movePower;

public:
	HRESULT InitSGwind(void);
	void UninitSGwind(void);
	void UpdateSGwind(void);
	//void DrawSGwind(void);

	void SetSGwind(void);
};
