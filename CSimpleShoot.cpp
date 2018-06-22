#include <stdio.h>
#include "CSimpleShoot.h"


// private section //

void CSimpleShoot::Shoot( float x )
{
    static DWORD s_dwShootCount = 0;

    m_pBullets[s_dwShootCount].bActive = true;
    m_pBullets[s_dwShootCount].x = x;
    m_pBullets[s_dwShootCount].y = 1.0f;
    m_pBullets[s_dwShootCount].z = -(PieceSize*(PieceLineNum-1)/2);

    CheckEnemy( m_pBullets[s_dwShootCount].x, m_pBullets[s_dwShootCount].z+PieceSize/2 );
    CheckEnemy( m_pBullets[s_dwShootCount].x, m_pBullets[s_dwShootCount].z );

    s_dwShootCount++;
    if( s_dwShootCount >= BulletsNum )
        s_dwShootCount = 0;
}

void CSimpleShoot::DrawBullets()
{
    D3DXMATRIX mat;

    for( int i=0; i < BulletsNum; i++ ){
        if( m_pBullets[i].bActive ){
            DrawBullet( D3DXMatrixTranslation(&mat,
                m_pBullets[i].x, m_pBullets[i].y, m_pBullets[i].z) );
        }
    }
}

void CSimpleShoot::MoveBullets( DWORD dwTime )
{
    const float c_fBulletMove = 0.1f;

    for( int i=0; i < BulletsNum; i++ ){
        if( m_pBullets[i].bActive ){
            if( m_pBullets[i].z >= (PieceSize*(PieceLineNum-1)/2) ){
                m_pBullets[i].bActive = false;
                continue;
            }
            m_pBullets[i].z += c_fBulletMove * dwTime;
            if( CheckEnemy(m_pBullets[i].x, m_pBullets[i].z+PieceSize/2) )
                m_pBullets[i].bActive = false;
            if( CheckEnemy(m_pBullets[i].x, m_pBullets[i].z) )
                m_pBullets[i].bActive = false;
        }
    }
}

void CSimpleShoot::DrawBullet( D3DMATRIX *pmatWorld )
{
    m_pcdx->m_pD3DDevice->SetTransform( D3DTS_WORLD, pmatWorld );

    m_pcdx->m_pD3DDevice->SetFVF( CDX::FVFVERTEX::FORMAT );
    m_pcdx->m_pD3DDevice->SetStreamSource( 0,
        m_pcdx->m_pD3DBulletVertex, 0, sizeof(CDX::FVFVERTEX) );
    m_pcdx->m_pD3DDevice->DrawPrimitive( D3DPT_LINELIST, 0, 1 );
}

void CSimpleShoot::CreateEnemy( DWORD kind, float x )
{
    DWORD dwEnemy = EnemyNum;

    for( int i=0; i < EnemyNum; i++ ){
        if( !m_pEnemy[i].bActive ){
            dwEnemy = i;
        }
    }
    if( dwEnemy == EnemyNum )
        return;

    m_pEnemy[dwEnemy].kind = kind;
    m_pEnemy[dwEnemy].bActive = true;
    m_pEnemy[dwEnemy].x = x;
    m_pEnemy[dwEnemy].y = 0.5f;
    m_pEnemy[dwEnemy].z = (PieceSize*(PieceLineNum-1)/2);
}

void CSimpleShoot::DrawEnemy()
{
    D3DXMATRIX mat;

    for( int i=0; i < EnemyNum; i++ ){
        if( m_pEnemy[i].bActive ){
            switch( m_pEnemy[i].kind ){
            case ENEMY1:
                DrawEnemy1( D3DXMatrixTranslation(&mat,
                    m_pEnemy[i].x, m_pEnemy[i].y, m_pEnemy[i].z) );
                break;
            }
        }
    }
}

void CSimpleShoot::MoveEnemy( DWORD dwTime )
{
    for( int i=0; i < EnemyNum; i++ ){
        if( m_pEnemy[i].bActive ){
            switch( m_pEnemy[i].kind ){
            case ENEMY1:
                MoveEnemy1( dwTime, &m_pEnemy[i] );
                break;
            }
        }
    }
}

bool CSimpleShoot::CheckEnemy( float x, float z )
{
    for( int i=0; i < EnemyNum; i++ ){
        if( m_pEnemy[i].bActive ){
            switch( m_pEnemy[i].kind ){
            case ENEMY1:
                if( CheckEnemy1(x, z, &m_pEnemy[i]) )
                    return true;
                break;
            }
        }
    }
    return false;
}

void CSimpleShoot::DrawEnemy1( D3DMATRIX *pmatWorld )
{
    m_pcdx->m_pD3DDevice->SetTransform( D3DTS_WORLD, pmatWorld );

    m_pcdx->m_pD3DDevice->SetFVF( CDX::FVFVERTEX::FORMAT );
    m_pcdx->m_pD3DDevice->SetStreamSource( 0,
        m_pcdx->m_pD3DEnemy1Vertex, 0, sizeof(CDX::FVFVERTEX) );
    m_pcdx->m_pD3DDevice->SetIndices( m_pcdx->m_pD3DEnemy1IBuffer );
    m_pcdx->m_pD3DDevice->DrawIndexedPrimitive( D3DPT_LINESTRIP, 0, 0, 8, 0, 15 );
}

void CSimpleShoot::MoveEnemy1( DWORD dwTime, CSimpleShoot::Enemy *pEnemy )
{
    const float c_fEnemyMove = 0.01f;

    pEnemy->z -= c_fEnemyMove * dwTime;
    if( pEnemy->z <= -(PieceSize*(PieceLineNum)/2) )
        m_bDead = true;
}

bool CSimpleShoot::CheckEnemy1( float x, float z, CSimpleShoot::Enemy *pEnemy )
{
    const DWORD c_dwAddPoint = 10;

    if( pEnemy->z >= z && (pEnemy->z-(float)PieceSize/2) <= z ){
        if( (pEnemy->x+(float)PieceSize/4) >= x && (pEnemy->x-(float)PieceSize/4) <= x ){
            pEnemy->bActive = false;
            m_pcdx->m_p3DBuffer->SetPosition(
                    pEnemy->x/(PieceSize*PieceLineNum-PieceSize)/2/5,
                    0.0f, pEnemy->z/(PieceSize*PieceLineNum-PieceSize)/2/5, DS3D_IMMEDIATE );
            m_pcdx->PlaySecondary( m_pcdx->m_pKilledE1Sound, m_pcdx->m_p3DAudioPath );
            m_dwPoint += c_dwAddPoint;
            return true;
        }
    }
    return false;
}

void CSimpleShoot::DrawPoint()
{
    RECT rect = { 0, 0, m_pcdx->m_D3DPP.BackBufferWidth, m_pcdx->m_D3DPP.BackBufferHeight };
    TCHAR szPoint[11];  // DWORD: 最大10桁

    wsprintf( szPoint, TEXT("%lu"), m_dwPoint );

    if( SUCCEEDED(m_pcdx->m_pD3DXSysFont->Begin()) ){
        m_pcdx->m_pD3DXSysFont->DrawText( szPoint, -1, &rect,
            DT_RIGHT | DT_TOP, D3DCOLOR_XRGB(0,0xFF,0xFF) );
        m_pcdx->m_pD3DXSysFont->End();
    }
}

void CSimpleShoot::DrawFPS()
{
    RECT rect = {0,0,0,0};

    if( SUCCEEDED(m_pcdx->m_pD3DXSysFont->Begin()) ){
        m_pcdx->m_pD3DXSysFont->DrawText( m_szFPS, -1, &rect,
            DT_NOCLIP, D3DCOLOR_XRGB(0xFF,0xFF,0xFF) );
        m_pcdx->m_pD3DXSysFont->End();
    }
}

void CSimpleShoot::DrawMap()
{
    D3DXMATRIX matWorld;

    D3DXMatrixIdentity( &matWorld );
    m_pcdx->m_pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );

    m_pcdx->m_pD3DDevice->SetFVF( CDX::FVFVERTEX::FORMAT );
    m_pcdx->m_pD3DDevice->SetStreamSource( 0,
        m_pcdx->m_pD3DMapVertex, 0, sizeof(CDX::FVFVERTEX) );
    m_pcdx->m_pD3DDevice->DrawPrimitive( D3DPT_LINELIST, 0, (PieceLineNum+1)*2 );
}

void CSimpleShoot::DrawPlane( D3DMATRIX *pmatWorld )
{
    m_pcdx->m_pD3DDevice->SetTransform( D3DTS_WORLD, pmatWorld );

    m_pcdx->m_pD3DDevice->SetFVF( CDX::FVFVERTEX::FORMAT );
    m_pcdx->m_pD3DDevice->SetStreamSource( 0,
        m_pcdx->m_pD3DPlaneVertex, 0, sizeof(CDX::FVFVERTEX) );
    m_pcdx->m_pD3DDevice->DrawPrimitive( D3DPT_LINESTRIP, 0, 3 );
}

void CSimpleShoot::InitTitle()
{
    m_Title_bBlink = true;
    m_dwPoint = 0;
    m_pcdx->PlayBGM( m_pcdx->m_pTitleBGM );
}

void CSimpleShoot::MoveTitle( DWORD dwTime )
{
    const DWORD c_GoTime = 1000;
    const bool c_bGo = false;
    const DWORD c_dwBlinkTime = 500;
    static DWORD s_dwTimeCount = 0;
    static DWORD s_GoCount = 0;
    static bool s_bGo = c_bGo;

    if( !s_bGo )
        s_GoCount += dwTime;
    if( !s_bGo && (s_GoCount >= c_GoTime) )
        s_bGo = true;
    if( s_bGo && m_pcdx->GetKeyState() ){
        if( m_pcdx->CheckKeyState(DIK_SPACE) ){
            s_dwTimeCount = 0;
            s_GoCount = 0;
            s_bGo = c_bGo;
            m_pcdx->PlaySecondary( m_pcdx->m_pStartSound );
            ChangeStage( STAGE_START );
            return;
        }
    }

    s_dwTimeCount += dwTime;
    if( s_dwTimeCount >= c_dwBlinkTime ){
        s_dwTimeCount -= c_dwBlinkTime;
        m_Title_bBlink = (s_bGo)?(!m_Title_bBlink):true;
    }
}

void CSimpleShoot::RenderTitle()
{
    RECT HightRect, LowRect;

    SetRect( &HightRect, 0, 0,
        m_pcdx->m_D3DPP.BackBufferWidth, m_pcdx->m_D3DPP.BackBufferHeight/2 );
    SetRect( &LowRect, 0, m_pcdx->m_D3DPP.BackBufferHeight/2,
        m_pcdx->m_D3DPP.BackBufferWidth, m_pcdx->m_D3DPP.BackBufferHeight );

    if( SUCCEEDED(m_pcdx->m_pD3DXTitleFont->Begin()) ){
        HightRect.left = HightRect.top = 32;
        m_pcdx->m_pD3DXTitleFont->DrawText( TEXT("SimpleShoot          "), -1, &HightRect,
            DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(0,0x7F,0xBF) );
        HightRect.left = HightRect.top = 0;
        m_pcdx->m_pD3DXTitleFont->DrawText( TEXT("SimpleShoot          "), -1, &HightRect,
            DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(0,0xBF,0xFF) );
        m_pcdx->m_pD3DXTitleFont->End();
    }

    if( SUCCEEDED(m_pcdx->m_pD3DXSysFont->Begin()) ){
        if( !m_Title_bBlink )
            m_pcdx->m_pD3DXSysFont->DrawText( TEXT("Push SpaceKey to Start!!"), -1, &LowRect,
                DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(0,0xFF,0) );
        m_pcdx->m_pD3DXSysFont->DrawText( TEXT("Copyright 流"), -1, &LowRect,
            DT_RIGHT | DT_BOTTOM, D3DCOLOR_XRGB(0,0xBF,0xBF) );
        m_pcdx->m_pD3DXSysFont->End();
    }
}

void CSimpleShoot::InitStart()
{
    D3DXMATRIX matProj;

    m_pcdx->StopSound( m_pcdx->m_pTitleBGM );

    double dMapHalfSize = PieceSize * (PieceLineNum) / 2 + 5;   // 5は実験値
    m_Start_dCamDis = (dMapHalfSize
        +sin(3.1415/6.0)*dMapHalfSize/tan(3.1415/(180.0/75.0)))/cos(3.1415/6.0);

    D3DXMatrixPerspectiveFovLH( &matProj, 3.1415f/2.0f,
        (FLOAT)m_pcdx->m_D3DPP.BackBufferWidth/m_pcdx->m_D3DPP.BackBufferHeight,
        0.0f, (FLOAT)m_Start_dCamDis );
    m_pcdx->m_pD3DDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

void CSimpleShoot::MoveStart( DWORD dwTime )
{
    const float c_fMovePos = 0.1f, c_fMoveLast = 6.0f;  // 30度(固定値)
    const DWORD c_dwMoveTime = 1000/30;
    const float c_Pos = 2.0f;
    static float s_Pos = c_Pos;
    static DWORD dwTimeCount = 0;
    D3DXMATRIX matView;
    D3DXVECTOR3 vAt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUp( 0.0f, 0.0f, 1.0f );
    D3DXVECTOR3 vEye;
    float fRad = 3.1415f / s_Pos;

    vEye.x = 0.0f;
    vEye.y = (FLOAT)sin( fRad ) * (FLOAT)m_Start_dCamDis;
    vEye.z = -(FLOAT)cos( fRad ) * (FLOAT)m_Start_dCamDis;

    D3DXMatrixLookAtLH( &matView, &vEye, &vAt, &vUp );
    m_pcdx->m_pD3DDevice->SetTransform( D3DTS_VIEW, &matView );

    dwTimeCount += dwTime;
    if( dwTimeCount >= c_dwMoveTime ){
        dwTimeCount -= c_dwMoveTime;
        if( s_Pos <= c_fMoveLast ){
            s_Pos += c_fMovePos;
        }else{
            s_Pos = c_Pos;
            dwTimeCount = 0;
            ChangeStage( STAGE_STAGE1 );
        }
    }
}

void CSimpleShoot::RenderStart()
{
    D3DXMATRIX mat;

    DrawMap();
    DrawPlane( D3DXMatrixTranslation(&mat, 0.0f, 0.0f, -(PieceSize*(PieceLineNum-1)/2)) );
}

void CSimpleShoot::InitStage1()
{
    m_bDead = false;
    m_bPause = false;

    m_Stage1_PosPlane.x = 0;
    m_Stage1_PosPlane.y = 0;
    m_Stage1_PosPlane.z = -(PieceSize*(PieceLineNum-1)/2);

    ZeroMemory( m_pBullets, sizeof(m_pBullets) );
    ZeroMemory( m_pEnemy, sizeof(m_pEnemy) );

    m_pcdx->m_pListener->SetRolloffFactor( 0.5f, DS3D_IMMEDIATE );
    m_pcdx->m_p3DBuffer->SetMinDistance( 0.1f, DS3D_IMMEDIATE );

    srand( timeGetTime() );

    m_pcdx->PlayBGM( m_pcdx->m_pStage1BGM );
}

void CSimpleShoot::MoveStage1( DWORD dwTime )
{
    const DWORD c_EnemyTime = 1000*2;
    const bool c_bEnemy = false, c_bBullet = true, c_bLShift = true;
    static DWORD s_EnemyTime = c_EnemyTime, s_EnemyCount = 0;
    static bool s_bEnemy = c_bEnemy;
    static DWORD s_BulletCount = 0;
    static bool s_bBullet = c_bBullet;
    static DWORD s_dwPauseCount;
    static bool s_bLShift = c_bLShift;
    const float c_fPlaneMove = 0.03f;
    const DWORD c_BulletTime = 1000/10;
    const DWORD c_EnemyTimeLimit = 300;
    const float c_EnemyTimeMove = 0.1f;
    const DWORD c_dwPauseTime = 1000;

    if( m_bDead ){
        s_EnemyTime = c_EnemyTime;
        s_EnemyCount = 0;
        s_bEnemy = c_bEnemy;
        s_BulletCount = 0;
        s_bBullet = c_bBullet;
        s_bLShift = c_bLShift;
        ChangeStage( STAGE_ENDING );
    }

    if( !s_bLShift ){
        s_dwPauseCount += dwTime;
        if( s_dwPauseCount >= c_dwPauseTime )
            s_bLShift = true;
    }
    if( m_pcdx->GetKeyState() ){
        if( s_bLShift && m_pcdx->CheckKeyState(DIK_LSHIFT) ){
            s_bLShift = false;
            s_dwPauseCount = 0;
            m_bPause = !m_bPause;
            m_pcdx->PlaySecondary( m_pcdx->m_pPauseSound );
        }
        if( m_bPause )
            return;
        if( m_pcdx->CheckKeyState(DIK_LEFT) ){
            m_Stage1_PosPlane.x -= c_fPlaneMove * dwTime;
            if( m_Stage1_PosPlane.x < -(PieceSize*PieceLineNum/2-PieceSize/2) )
                m_Stage1_PosPlane.x = -(PieceSize*PieceLineNum/2-PieceSize/2);
        }
        if( m_pcdx->CheckKeyState(DIK_RIGHT) ){
            m_Stage1_PosPlane.x += c_fPlaneMove * dwTime;
            if( m_Stage1_PosPlane.x > (PieceSize*PieceLineNum/2-PieceSize/2) )
                m_Stage1_PosPlane.x = (PieceSize*PieceLineNum/2-PieceSize/2);
        }
        if( m_pcdx->CheckKeyState(DIK_SPACE) ){
            if( s_bBullet ){
                s_bBullet = false;
                Shoot( m_Stage1_PosPlane.x );
                m_pcdx->m_p3DBuffer->SetPosition(
                    m_Stage1_PosPlane.x/(PieceSize*PieceLineNum/2-PieceSize/2)/5,
                    0.0f, 0.0f, DS3D_IMMEDIATE );
                m_pcdx->PlaySecondary( m_pcdx->m_pShootSound, m_pcdx->m_p3DAudioPath );
            }else{
                s_BulletCount += dwTime;
                if( s_BulletCount >= c_BulletTime ){
                    s_BulletCount = 0;
                    s_bBullet = true;
                }
            }
        }
    }else
        return;

    MoveBullets( dwTime );
    MoveEnemy( dwTime );

    if( !(s_EnemyTime <= c_EnemyTimeLimit) )
            s_EnemyTime -= (DWORD)(c_EnemyTimeMove * dwTime);
    s_EnemyCount += dwTime;
    if( s_EnemyCount >= s_EnemyTime ){
        s_EnemyCount -= s_EnemyTime;
        s_bEnemy = true;
    }
    if( s_bEnemy ){
        s_bEnemy = false;
        float PosEnemy = (float)(rand()%(PieceSize*PieceLineNum-PieceSize));
        if( PosEnemy > (PieceSize*PieceLineNum-PieceSize)/2 )
            PosEnemy = -(PosEnemy-(PieceSize*PieceLineNum-PieceSize)/2);
        CreateEnemy( ENEMY1, PosEnemy );
    }
}

void CSimpleShoot::RenderStage1()
{
    const FLOAT c_fPlainRotationRad = 3.1415f/6.0f;
    D3DXMATRIX matWorld, mat;
    FLOAT fPlaneRotation = 0;

    DrawMap();
    DrawBullets();
    DrawEnemy();

    if( !m_bPause && m_pcdx->GetKeyState() ){
        if( !(m_pcdx->CheckKeyState(DIK_LEFT) && m_pcdx->CheckKeyState(DIK_RIGHT))
                && (m_pcdx->CheckKeyState(DIK_LEFT) || m_pcdx->CheckKeyState(DIK_RIGHT)) ){
            fPlaneRotation = m_pcdx->CheckKeyState(DIK_LEFT)?
                c_fPlainRotationRad:-c_fPlainRotationRad;
        }
    }
    D3DXMatrixRotationZ( &matWorld, fPlaneRotation );
    matWorld *= *D3DXMatrixTranslation( &mat,
        m_Stage1_PosPlane.x, m_Stage1_PosPlane.y, m_Stage1_PosPlane.z );
    DrawPlane( &matWorld );

    DrawPoint();

    RECT rect = {0,0,0,0};

    if( m_bPause && SUCCEEDED(m_pcdx->m_pD3DXSysFont->Begin()) ){
        RECT rect = { 0, 0, m_pcdx->m_D3DPP.BackBufferWidth, m_pcdx->m_D3DPP.BackBufferHeight };
        m_pcdx->m_pD3DXSysFont->DrawText( TEXT("PAUSE"), -1, &rect,
            DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(0xFF,0xFF,0xFF) );
        m_pcdx->m_pD3DXSysFont->End();
    }
}

void CSimpleShoot::InitEnding()
{
    m_pcdx->PlaySoundWithBGM( m_pcdx->m_pEndingSound, m_pcdx->m_pEndingBGM );
    m_Ending_color = D3DCOLOR_XRGB(0,0,0);
    srand( timeGetTime() );
}

void CSimpleShoot::MoveEnding( DWORD dwTime )
{
    const DWORD c_dwWaitTime = 1000/30;
    const DWORD c_dwGoBackTime = 5000;
    const bool c_bGoBack = false;
    static DWORD s_dwTimeCount = 0, s_dwGoBackCount = 0;
    static bool s_bGoBack = c_bGoBack;

    if( !s_bGoBack )
        s_dwGoBackCount += dwTime;
    if( !s_bGoBack && (s_dwGoBackCount >= c_dwGoBackTime) )
        s_bGoBack = true;
    if( s_bGoBack && m_pcdx->GetKeyState() ){
        if( m_pcdx->CheckKeyState(DIK_SPACE) ){
            s_dwTimeCount = 0;
            s_dwGoBackCount = 0;
            s_bGoBack = c_bGoBack;
            ChangeStage( STAGE_TITLE );
            return;
        }
    }

    s_dwTimeCount += dwTime;
    if( s_dwTimeCount >= c_dwWaitTime ){
        s_dwTimeCount -= c_dwWaitTime;
        m_Ending_color = D3DCOLOR_XRGB( 0, rand()%0xFF, rand()&0xFF );
    }
}

void CSimpleShoot::RenderEnding()
{
    RECT rect = { 0, 0, m_pcdx->m_D3DPP.BackBufferWidth, m_pcdx->m_D3DPP.BackBufferHeight };

    if( SUCCEEDED(m_pcdx->m_pD3DXTitleFont->Begin()) ){
        m_pcdx->m_pD3DXTitleFont->DrawText( TEXT("GAME OVER..."), -1, &rect,
            DT_CENTER | DT_VCENTER, m_Ending_color );
        m_pcdx->m_pD3DXTitleFont->End();
    }

    DrawPoint();
}


// public section //

CSimpleShoot::CSimpleShoot( LPCDX pcdx )
:   m_Stage(0), m_pcdx(pcdx)
{
    m_szFPS[0] = '\0';
}

void CSimpleShoot::ChangeStage( DWORD Stage )
{
    m_Stage = Stage;

    switch( m_Stage ){
    case STAGE_TITLE:
        InitTitle();
        MoveTitle( 0 );
        break;
    case STAGE_START:
        InitStart();
        MoveStart( 0 );
        break;
    case STAGE_STAGE1:
        InitStage1();
        MoveStage1( 0 );
        break;
    case STAGE_ENDING:
        InitEnding();
        MoveEnding( 0 );
        break;
    }
}

void CSimpleShoot::FrameMove()
{
    static DWORD s_dwTimeBefore = timeGetTime();
    DWORD dwTimeNow, dwTimeDifference;

    // 前回の呼び出しから何ms経過したかチェック
    dwTimeNow = timeGetTime();
    if( s_dwTimeBefore > dwTimeNow )
        dwTimeDifference = 0;
    else
        dwTimeDifference = dwTimeNow - s_dwTimeBefore;
    s_dwTimeBefore = dwTimeNow;

    // FPSの作成
    const c_FPSDrawTime = 100;
    static s_dwTimeCount = 0;
    s_dwTimeCount += dwTimeDifference;
    if( s_dwTimeCount >= c_FPSDrawTime ){
        s_dwTimeCount -= c_FPSDrawTime;
        double FPS = (double)1000 / dwTimeDifference;
    #ifdef UNICODE
        swprintf( (wchar_t *)m_szFPS, L"FPS: %3.1f", FPS );
    #else
        sprintf( (char *)m_szFPS, "FPS: %3.1f", FPS );
    #endif
    }

    if( m_pcdx->GetKeyState() ){
        if( m_pcdx->CheckKeyState(DIK_ESCAPE) ){
            DestroyWindow( m_pcdx->m_hWnd );
            return;
        }
    }

    switch( m_Stage ){
    case STAGE_TITLE:
        MoveTitle( dwTimeDifference );
        break;
    case STAGE_START:
        MoveStart( dwTimeDifference );
        break;
    case STAGE_STAGE1:
        MoveStage1( dwTimeDifference );
        break;
    case STAGE_ENDING:
        MoveEnding( dwTimeDifference );
        break;
    }
}

void CSimpleShoot::Render()
{
    DrawFPS();

    switch( m_Stage ){
    case STAGE_TITLE:
        RenderTitle();
        break;
    case STAGE_START:
        RenderStart();
        break;
    case STAGE_STAGE1:
        RenderStage1();
        break;
    case STAGE_ENDING:
        RenderEnding();
        break;
    }
}
