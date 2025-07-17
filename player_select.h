#pragma once

HRESULT InitPlayerSelect(void);
void UninitPlayerSelect(void);
void UpdatePlayerSelect(void);
void DrawPlayerSelect(void);

void SetSelectedCharIndex(int playerIndex, int charIndex);
int GetSelectedCharIndex(int playerIndex);
