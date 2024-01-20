// CSimpleShoot
//  SimpleShootメインエンジン
//  LINK: winmm.lib

#ifndef _CSimpleShoot_
#define _CSimpleShoot_

#include <windows.h>
#include "CDX.h"


#define BulletsNum      (10)
#define EnemyNum        (10)

#define STAGE_TITLE     (0x00000001)
#define STAGE_START     (0x00000002)
#define STAGE_STAGE1    (0x00000003)
#define STAGE_ENDING    (0x00000004)

#define ENEMY1  (0x00000001)


class CSimpleShoot
{
    DWORD m_Stage;
    LPCDX m_pcdx;
    TCHAR m_szFPS[11];
    DWORD m_dwPoint;

    struct Bullet
    {
        bool bActive;
        float x, y, z;
    } m_pBullets[BulletsNum];
    struct Enemy
    {
        DWORD kind;
        bool bActive;
        float x, y, z;
    } m_pEnemy[EnemyNum];

    void Shoot( float x );
    void DrawBullets();
    void MoveBullets( DWORD dwTime );
    void DrawBullet( D3DMATRIX *pmatWorld );

    void CreateEnemy( DWORD kind, float x );
    void DrawEnemy();
    void MoveEnemy( DWORD dwTime );
    bool CheckEnemy( float x, float z );

    void DrawEnemy1( D3DMATRIX *pmatWorld );
    void MoveEnemy1( DWORD dwTime, Enemy *pEnemy );
    bool CheckEnemy1( float x, float z, Enemy *pEnemy );

    void DrawPoint();
    void DrawFPS();
    void DrawMap();
    void DrawPlane( D3DMATRIX *pmatWorld );

    bool m_Title_bBlink;
    void InitTitle();
    void MoveTitle( DWORD dwTime );
    void RenderTitle();

    double m_Start_dCamDis;
    void InitStart();
    void MoveStart( DWORD dwTime );
    void RenderStart();

    bool m_bDead;
    bool m_bPause;
    D3DVECTOR m_Stage1_PosPlane;
    void InitStage1();
    void MoveStage1( DWORD dwTime );
    void RenderStage1();

    D3DCOLOR m_Ending_color;
    void InitEnding();
    void MoveEnding( DWORD dwTime );
    void RenderEnding();

public:
    CSimpleShoot( LPCDX pcdx );

    void ChangeStage( DWORD Stage );
    void FrameMove();
    void Render();
};
typedef CSimpleShoot    *PCSimpleShoot, *LPCSimpleShoot;

#endif  // _CSimpleShoot_
