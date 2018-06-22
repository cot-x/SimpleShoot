// CDX
//  DirectX基本処理クラス
//  LINK: d3d9.lib d3dx9.lib dinput8.lib dxguid.lib

#ifndef _CDX_
#define _CDX_

#include <windows.h>
#include <d3dx9.h>
#include <dinput.h>
#include <dmusici.h>


#define RELEASE(x)  if(x){ x->Release(); x=NULL; }

#if defined(DEBUG) || defined(_DEBUG)
#define DebugMessage(str, hWnd) MessageBox( hWnd, TEXT(str), TEXT("DEBUG"), MB_ICONINFORMATION )
#else
#define DebugMessage(str, hWnd) (0)
#endif


class CDX
{
    bool m_bInit, m_bDeviceLost, m_bSetViewport, m_bMusicCOM;
    HINSTANCE m_hInstance;
    LPDIRECT3D9 m_pD3D;
    LPDIRECTINPUT8 m_pDInput;
    BYTE m_pKeyState[256];

    // DirectX Graphicsの初期化
    bool InitDXGraphics();
    // DirectInputの初期化
    bool InitDInput();
    // DirectMusicの初期化
    bool InitDMusic();

    // DirectX Graphicsの終了処理
    void CleanupDXGraphics();
    // DirectInputの終了処理
    void CleanupDInput();
    // DirectMusicの終了処理
    void CleanupDMusic();

    // ステートのセット
    void SetState();
    // その他オブジェクトの確保
    bool InitObject();
    // その他オブジェクトの開放
    void CleanupObject();
    // DirectX Graphicsに管理されないオブジェクトのクリーンアップ
    void CleanupD3DObject();
    // DirectX Graphicsに管理されないオブジェクトの初期化
    void InitD3DObject();

public:
    HWND m_hWnd;
    D3DPRESENT_PARAMETERS m_D3DPP;
    LPDIRECT3DDEVICE9 m_pD3DDevice;
    LPDIRECTINPUTDEVICE8 m_pDIDevice;
    IDirectMusicPerformance8 *m_pPerformance;
    IDirectMusicLoader8 *m_pLoader;

    // その他オブジェクト
    #define PieceSize       (5)
    #define PieceLineNum    (10)
    #define MapLineColor    D3DCOLOR_XRGB(0,0xFF,0)
    #define PlaneLineColor  D3DCOLOR_XRGB(0,0xFF,0xFF)
    #define BulletLineColor D3DCOLOR_XRGB(0xFF,0xFF,0x7F)
    #define Enemy1LineColor  D3DCOLOR_XRGB(0xFF,0,0)
    struct FVFVERTEX
    {
        float x, y, z;
        D3DCOLOR color;
        enum{ FORMAT = D3DFVF_XYZ | D3DFVF_DIFFUSE };
    };
    IDirectMusicAudioPath8 *m_p3DAudioPath;
    LPDIRECTSOUND3DLISTENER8 m_pListener;
    LPDIRECTSOUND3DBUFFER8 m_p3DBuffer;
    LPD3DXFONT m_pD3DXSysFont;
    LPD3DXFONT m_pD3DXTitleFont;
    IDirectMusicSegment8 *m_pStartSound;
    IDirectMusicSegment8 *m_pShootSound;
    IDirectMusicSegment8 *m_pKilledE1Sound;
    IDirectMusicSegment8 *m_pEndingSound;
    IDirectMusicSegment8 *m_pPauseSound;
    IDirectMusicSegment8 *m_pTitleBGM;
    IDirectMusicSegment8 *m_pStage1BGM;
    IDirectMusicSegment8 *m_pEndingBGM;
    LPDIRECT3DVERTEXBUFFER9 m_pD3DMapVertex;
    LPDIRECT3DVERTEXBUFFER9 m_pD3DPlaneVertex;
    LPDIRECT3DVERTEXBUFFER9 m_pD3DBulletVertex;
    LPDIRECT3DVERTEXBUFFER9 m_pD3DEnemy1Vertex;
    LPDIRECT3DINDEXBUFFER9 m_pD3DEnemy1IBuffer;


    // コンストラクタ
    CDX( HWND hWnd );
    // デストラクタ
    virtual ~CDX();

    CDX( const CDX & ){}    // コピーコンストラクタ(無効)
    CDX & operator =( const CDX & ){ return *this; }  // 代入オペレータ(無効)

    // インスタンスが正常に作成できたかチェック
    inline bool CheckInit(){ return m_bInit; }
    // ビューポートが正常にセットされたかチェック
    inline bool CheckSetViewport(){ return m_bSetViewport; }
    // デバイスが消失してるかチェック
    inline bool CheckDeviceLost(){ return m_bDeviceLost; }
    // デバイス消失フラグをオン
    inline void SetDeviceLost(){ m_bDeviceLost = true; }

    // ビューポートのセット
    void SetViewport();
    // 消失したデバイスのリセット
    void DeviceLost();

    // キーボードの状態を取得(pKeyStateは256バイトの配列を渡す)
    bool GetKeyState();
    // GetKeyStateで取得した情報の検証
    bool CheckKeyState( DWORD dwKeyState );

    // マスターボリュームの設定
    bool SetMasterVolume( long lVolume );
    // BGMを再生(ループ有り)
    bool PlayBGM( IDirectMusicSegment8 *pSource, IUnknown *pAudioPath = NULL );
    // pSource1を再生したあとpSource2を再生
    bool PlaySoundWithBGM( IDirectMusicSegment8 *pSource1,
        IDirectMusicSegment8 *pSource2, IUnknown *pAudioPath = NULL );
    // セカンダリバッファで再生(ループ無し)
    bool PlaySecondary( IDirectMusicSegment8 *pSource, IUnknown *pAudioPath = NULL );
    // 再生停止
    bool StopSound( IUnknown *pObjectToStop );
    // 全再生停止
    bool StopSoundALL();
};
typedef CDX *PCDX, *LPCDX;

#endif  // _CDX_
