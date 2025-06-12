#pragma once


// �X�e�[�W�I����ʂ̊֐��錾
HRESULT InitStageSelect(void);    // ������
void UninitStageSelect(void);     // �I������
void UpdateStageSelect(void);     // �X�V����
void DrawStageSelect(void);       // �`�揈��

// 3D�X�e�[�W�I��p�̍\����
struct STAGE_SELECT_3D {
    XMFLOAT3 position;   // �X�e�[�W�̈ʒu
    XMFLOAT3 rotation;   // �X�e�[�W�̉�]
    float scale;         // �X�e�[�W�̃X�P�[��
    int stageId;         // �X�e�[�WID
    bool isSelected;     // �I�𒆂��ǂ���
};