#include "main.h"
#include "input.h"
#include "SG_wind.h"
#include "renderer.h"
#include "player.h"

HRESULT WIND::InitSGwind(void)
{
    use = FALSE;
    movePower = XMFLOAT3(0.0f, 0.0f, 0.0f);
    countTime = 0;
    spawnTime = GetRand(100, 120);
    spawnCount = 0;
    return S_OK;
}


void WIND::UninitSGwind(void)
{
    use = FALSE;
    movePower = XMFLOAT3(0.0f, 0.0f, 0.0f);
    countTime = 0;
    spawnTime = 0;
    spawnCount = 0;
}

void WIND::UpdateSGwind(void)
{
    if (use)
    {
        if (countTime > 10)FinishSGwind();

        for (int i = 0; i < MAX_PLAYER; i++)
        {
            PLAYER* player = GetPlayer(i);

            player->pos.x += movePower.x;
            player->pos.z += movePower.z;
        }

        countTime++;
    }
    else if(!use)
    {
        spawnCount++;
        if (spawnCount > spawnTime)
        {
            SetSGwind();
        }
    }
}

void WIND::SetSGwind(void)
{
    use = TRUE;
    countTime = 0;
    movePower.x = GetRand(-1, 1);
    movePower.z = GetRand(-1, 1);
}

void WIND::FinishSGwind(void)
{
    use = FALSE;
    countTime = 0;
    spawnTime = GetRand(100, 120);
    spawnCount = 0;
}
