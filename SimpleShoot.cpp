// SimpleShoot
//  Ver.1.0.0a  <2003/05/07>    企画･開発開始
//  Ver.1.0.0   <2003/05/10>    Ver.1.0.0完成 (開発ほぼ終了)
//      Copyright: Yuki
#define APPNAME TEXT("SimpeShoot Ver.1.0.0")

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "CDX.h"
#include "CSimpleShoot.h"


#define BUFWIDTH        (640)
#define BUFHEIGHT       (480)
#define MASTER_VOLUME   (1000)


LRESULT CALLBACK WndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
void Active();
bool GetKeyState( BYTE *pKeyState );
bool CheckKeyState( BYTE *pKeyState, DWORD dwKeyState );


HWND g_hWnd;
bool g_bActive;
LPCDX g_pcdx = NULL;
LPCSimpleShoot g_pss;


// エントリーポイント
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int )
{
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    WNDCLASSEX wc;
    RECT rect;
    MSG msg;

    wc.cbSize           = sizeof(wc);
    wc.style            = 0;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon( hInstance, TEXT("MAIN_ICON") );
    wc.hIconSm          = NULL;
    wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground    = CreateSolidBrush( RGB(0,0,0) );
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = APPNAME;
    if( !RegisterClassEx(&wc) ){
        MessageBox( NULL, TEXT("ウィンドウクラスの登録に失敗しました。"),
            TEXT("RegisterClassEx"), MB_ICONERROR );
        return 0;
    }

    SetRect( &rect, 0, 0, BUFWIDTH, BUFHEIGHT );
    AdjustWindowRect( &rect, dwStyle, FALSE );
    g_hWnd = CreateWindow( APPNAME, APPNAME, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right, rect.bottom, NULL, NULL, hInstance, NULL );
    if( !g_hWnd ){
        MessageBox( NULL, TEXT("ウィンドウの作成に失敗しました。"),
            TEXT("CreateWindow"), MB_ICONERROR );
        return 0;
    }

    CDX cdx( g_hWnd );
    g_pcdx = &cdx;
    if( !g_pcdx->CheckInit() ){
        MessageBox( g_hWnd, TEXT("DirectXの初期化に失敗しました。"),
            TEXT("CDX"), MB_ICONERROR );
        return 0;
    }
    g_pcdx->m_pDIDevice->Acquire();
    g_pcdx->SetMasterVolume( MASTER_VOLUME );

    CSimpleShoot ss( g_pcdx );
    g_pss = &ss;
    g_pss->ChangeStage( STAGE_TITLE );

    ShowWindow( g_hWnd, SW_SHOW );

    do{
        if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ){
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }else if( g_pcdx->CheckDeviceLost() ){
            g_pcdx->DeviceLost();
        }else if( !g_pcdx->CheckSetViewport() ){
            g_pcdx->SetViewport();
        }else if( g_bActive ){
            Active();
        }else{
            WaitMessage();
        }
    }while( msg.message != WM_QUIT );

    return msg.wParam;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
    switch( Msg ){
    case WM_ACTIVATE:
        if( g_pcdx && g_pcdx->CheckInit() && LOWORD(wParam) != WA_INACTIVE )
            g_pcdx->m_pDIDevice->Acquire();
        break;

    case WM_SIZE:
        if( wParam == SIZE_MINIMIZED )
            g_bActive = false;
        else
            g_bActive = true;
        return 0;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;
    }

    return DefWindowProc( hWnd, Msg, wParam, lParam );
}

// メインルーチン
void Active()
{
    HRESULT hr;

    g_pss->FrameMove();

    g_pcdx->m_pD3DDevice->Clear( 0, NULL,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );

    if( SUCCEEDED(g_pcdx->m_pD3DDevice->BeginScene()) ){
        g_pss->Render();
        g_pcdx->m_pD3DDevice->EndScene();
    }

    hr = g_pcdx->m_pD3DDevice->Present( NULL, NULL, NULL, NULL );
    if( hr == D3DERR_DEVICELOST ){
        g_pcdx->SetDeviceLost();
    }else if( hr == D3DERR_DRIVERINTERNALERROR ){
        MessageBox( g_hWnd, TEXT("内部ドライバエラーが発生しました。"),
            TEXT("IDirect3DDevice9::Present"), MB_ICONERROR );
        DestroyWindow( g_hWnd );
    }
}
