#include "main.h"
#include "input.h"
#include "SG_wind.h"
#include "renderer.h"
#include "player.h"

HRESULT WIND::InitSGwind(void)
{
    use = FALSE;
    movePower = XMFLOAT3(1.0f, 0.0f, 0.0f);
    return S_OK;
}


void WIND::UninitSGwind(void)
{
    use = FALSE;
}

void WIND::UpdateSGwind(void)
{
    if (use)
    {
        PLAYER* player = GetPlayer();

        player->pos.x += movePower.x;
        player->pos.z += movePower.z;
    }
}

void WIND::SetSGwind(void)
{
    use = FALSE;
}