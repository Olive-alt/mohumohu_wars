#pragma once

// ���[�h�I����ʂ̊֐��錾
HRESULT InitModeSelect(void);    // ������
void UninitModeSelect(void);     // �I������
void UpdateModeSelect(void);     // �X�V����
void DrawModeSelect(void);       // �`�揈��

// ���[�h�I��p�̍\���́i�����I�Ɋg���ł���悤�ɗp�Ӂj
struct MODE_SELECT_INFO {
    int modeId;                 // ���[�hID
    const char* modeName;       // ���[�h��
    bool isSelected;            // ���ݑI�𒆂�
};
