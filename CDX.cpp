#include "CDX.h"


// private section //

// DirectX Graphicsの初期化
bool CDX::InitDXGraphics()
{
    const D3DFORMAT DepthFormat = D3DFMT_D16;
    HRESULT hr;

    m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( !m_pD3D ){
        MessageBox( m_hWnd, TEXT("IDirect3D9の取得に失敗しました。"),
            TEXT("Direct3DCreate9"), MB_ICONERROR );
        return false;
    }

    ZeroMemory( &m_D3DPP, sizeof(m_D3DPP) );
    m_D3DPP.Windowed                = TRUE;
    m_D3DPP.SwapEffect              = D3DSWAPEFFECT_DISCARD;
    m_D3DPP.BackBufferFormat        = D3DFMT_UNKNOWN;
    m_D3DPP.EnableAutoDepthStencil  = TRUE;
    m_D3DPP.AutoDepthStencilFormat  = DepthFormat;
    hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
        D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_D3DPP, &m_pD3DDevice );
    if( FAILED(hr) ){
        hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_D3DPP, &m_pD3DDevice );
        if( FAILED(hr) ){
            hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, m_hWnd,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_D3DPP, &m_pD3DDevice );
            if( FAILED(hr) ){
                m_pD3DDevice = NULL;
                MessageBox( m_hWnd, TEXT("IDirect3DDevice9の取得に失敗しました。"),
                    TEXT("IDirect3D9::CreateDevice"), MB_ICONERROR );
                return false;
            }else
                DebugMessage( "RELで実行します。", m_hWnd );
        }else
            DebugMessage( "SORTWARE HALで実行します。", m_hWnd );
    }else
        DebugMessage( "PURE HARDWARE HALで実行します。", m_hWnd );

    SetViewport();
    SetState();

    return true;
}

// DirectInputの初期化
bool CDX::InitDInput()
{
    HRESULT hr;

    hr = DirectInput8Create( m_hInstance,
        DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDInput, NULL );
    if( FAILED(hr) ){
        m_pDInput = NULL;
        MessageBox( m_hWnd, TEXT("IDirectInput8の取得に失敗しました。"),
            TEXT("DirectInput8Create"), MB_ICONERROR );
        return false;
    }

    hr = m_pDInput->CreateDevice( GUID_SysKeyboard, &m_pDIDevice, NULL );
    if( FAILED(hr) ){
        m_pDIDevice = NULL;
        MessageBox( m_hWnd, TEXT("IDirectInputDevice8の取得に失敗しました。"),
            TEXT("IDirectInput8::CreateDevice"), MB_ICONERROR );
        return false;
    }

    hr = m_pDIDevice->SetDataFormat( &c_dfDIKeyboard );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("データフォーマットの設定に失敗しました。"),
            TEXT("IDirectInputDevice8::SetDataFormat"), MB_ICONERROR );
        return false;
    }

    hr = m_pDIDevice->SetCooperativeLevel( m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("協調モードの設定に失敗しました。"),
            TEXT("IDirectInputDevice8::SetCooperativeLevel"), MB_ICONERROR );
        return false;
    }

    return true;
}

// DirectMusicの初期化
bool CDX::InitDMusic()
{
    HRESULT hr;

    hr = CoInitialize( NULL );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("DirectMusicでCOMの初期化に失敗しました。"),
            TEXT("CoInitialize"), MB_ICONERROR );
        return false;
    }
    m_bMusicCOM = true;

    hr = CoCreateInstance( CLSID_DirectMusicPerformance, NULL,
        CLSCTX_INPROC, IID_IDirectMusicPerformance8, (void **)&m_pPerformance );
    if( FAILED(hr) ){
        m_pPerformance = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicPerformance8の取得に失敗しました。"),
            TEXT("CoCreateInstance"), MB_ICONERROR );
        return false;
    }

    hr = m_pPerformance->InitAudio( NULL, NULL, m_hWnd,
        DMUS_APATH_SHARED_STEREOPLUSREVERB, 64, DMUS_AUDIOF_ALL, NULL );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("パフォーマンスの初期化に失敗しました。"),
            TEXT("IDirectMusicPerformance8::InitAudio"), MB_ICONERROR );
        return false;
    }

    hr = CoCreateInstance( CLSID_DirectMusicLoader, NULL,
        CLSCTX_INPROC, IID_IDirectMusicLoader8, (void **)&m_pLoader );
    if( FAILED(hr) ){
        m_pLoader = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicLoader8の取得に失敗しました。"),
            TEXT("CoCreateInstance"), MB_ICONERROR );
        return false;
    }

    return true;
}

// DirectX Graphicsの終了処理
void CDX::CleanupDXGraphics()
{
    RELEASE( m_pD3DDevice );
    RELEASE( m_pD3D );
}

// DirectInputの終了処理
void CDX::CleanupDInput()
{
    RELEASE( m_pDIDevice );
    RELEASE( m_pDInput );
}

// DirectMusicの終了処理
void CDX::CleanupDMusic()
{
    if( m_pPerformance )
        m_pPerformance->Stop( NULL, NULL, 0, 0 );
    RELEASE( m_pLoader );
    if( m_pPerformance )
        m_pPerformance->CloseDown();
    RELEASE( m_pPerformance );

    if( m_bMusicCOM )
        CoUninitialize();
}

// ステートのセット
void CDX::SetState()
{
    m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );


    m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
}

// その他オブジェクトの確保
bool CDX::InitObject()
{
    HRESULT hr;
    HRSRC hFound;
    HGLOBAL hResource;
    DMUS_OBJECTDESC ObjDesc;
    FVFVERTEX *pBuffer;
    WORD *pIndex;

    // m_p3DAudioPath
    hr = m_pPerformance->CreateStandardAudioPath(
        DMUS_APATH_DYNAMIC_3D, 64, TRUE, &m_p3DAudioPath );
    if( FAILED(hr) ){
        m_p3DAudioPath = NULL;
        MessageBox( m_hWnd, TEXT("3D用IDirectMusicAudioPathの取得に失敗しました。"),
            TEXT("IDirectMusicPerformance8::CreateStandardAudioPath"), MB_ICONERROR );
        return false;
    }

    // m_pListener
    hr = m_p3DAudioPath->GetObjectInPath( 0, DMUS_PATH_PRIMARY_BUFFER, 0,
        GUID_NULL, 0, IID_IDirectSound3DListener, (void **)&m_pListener );
    if( FAILED(hr) ){
        m_pListener = NULL;
        MessageBox( m_hWnd, TEXT("IDirectSound3DListenerの取得に失敗しました。"),
            TEXT("IDirectMusicAudioPath8::GetObjectInPath"), MB_ICONERROR );
        return false;
    }

    // m_p3DBuffer
    hr = m_p3DAudioPath->GetObjectInPath( DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER,
        0, GUID_NULL, 0, IID_IDirectSound3DBuffer, (void **)&m_p3DBuffer );
    if( FAILED(hr) ){
        m_p3DBuffer = NULL;
        MessageBox( m_hWnd, TEXT("IDirectSound3DBufferの取得に失敗しました。"),
            TEXT("IDirectMusicAudioPath8::GetObjectInPath"), MB_ICONERROR );
        return false;
    }

    // m_pD3DXSysFont
    hr = D3DXCreateFont( m_pD3DDevice, (HFONT)GetStockObject(SYSTEM_FONT), &m_pD3DXSysFont );
    if( FAILED(hr) ){
        m_pD3DXSysFont = NULL;
        MessageBox( m_hWnd, TEXT("ID3DXFontの取得に失敗しました。(システムフォント)"),
            TEXT("D3DXCreateFont"), MB_ICONERROR );
        return false;
    }

    // m_pD3DXTitleFont
    LOGFONT lf;
    lf.lfHeight         = 64;                   lf.lfWidth          = 0;
    lf.lfEscapement     = 0;                    lf.lfOrientation    = 0;
    lf.lfWeight         = FW_BOLD;              lf.lfItalic         = TRUE;
    lf.lfUnderline      = TRUE;                 lf.lfStrikeOut      = FALSE;
    lf.lfCharSet        = SHIFTJIS_CHARSET;     lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;  lf.lfQuality        = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH || FF_DONTCARE;
    lstrcpy( lf.lfFaceName, TEXT("") );
    hr = D3DXCreateFontIndirect( m_pD3DDevice, &lf, &m_pD3DXTitleFont );
    if( FAILED(hr) ){
        m_pD3DXSysFont = NULL;
        MessageBox( m_hWnd, TEXT("ID3DXFontの取得に失敗しました。(タイトル用フォント)"),
            TEXT("D3DXCreateFontIndirect"), MB_ICONERROR );
        return false;
    }

    // m_pStartSound
    hFound = FindResource( NULL, TEXT("START_WAVE"), TEXT("WAVE") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.dwSize      = sizeof(ObjDesc);
    ObjDesc.guidClass   = CLSID_DirectMusicSegment;
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pStartSound );
    if( FAILED(hr) ){
        m_pStartSound = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(START_WAVE)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pStartSound->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(START_WAVE)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pShootSound
    hFound = FindResource( NULL, TEXT("SHOOT_WAVE"), TEXT("WAVE") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.dwSize      = sizeof(ObjDesc);
    ObjDesc.guidClass   = CLSID_DirectMusicSegment;
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pShootSound );
    if( FAILED(hr) ){
        m_pShootSound = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(SHOOT_WAVE)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pShootSound->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(SHOOT_WAVE)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pKilledE1Sound
    hFound = FindResource( NULL, TEXT("KILLEDE1_WAVE"), TEXT("WAVE") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.dwSize      = sizeof(ObjDesc);
    ObjDesc.guidClass   = CLSID_DirectMusicSegment;
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pKilledE1Sound );
    if( FAILED(hr) ){
        m_pKilledE1Sound = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(KILLEDE1_WAVE)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pKilledE1Sound->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(KILLEDE1_WAVE)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pEndingSound
    hFound = FindResource( NULL, TEXT("ENDING_WAVE"), TEXT("WAVE") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.dwSize      = sizeof(ObjDesc);
    ObjDesc.guidClass   = CLSID_DirectMusicSegment;
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pEndingSound );
    if( FAILED(hr) ){
        m_pEndingSound = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(ENDING_WAVE)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pEndingSound->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(ENDING_WAVE)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pPauseSound
    hFound = FindResource( NULL, TEXT("PAUSE_WAVE"), TEXT("WAVE") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.dwSize      = sizeof(ObjDesc);
    ObjDesc.guidClass   = CLSID_DirectMusicSegment;
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pPauseSound );
    if( FAILED(hr) ){
        m_pPauseSound = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(PAUSE_WAVE)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pPauseSound->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(PAUSE_WAVE)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pTitleBGM
    hFound = FindResource( NULL, TEXT("TITLE_MIDI"), TEXT("MIDI") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pTitleBGM );
    if( FAILED(hr) ){
        m_pTitleBGM = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(TITLE_MIDI)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pTitleBGM->SetParam( GUID_StandardMIDIFile, 0xFFFFFFFF, 0, 0, NULL );
    if( FAILED(hr) ){
        MessageBox( m_hWnd,
            TEXT("スタンダードMIDIファイルの初期化に失敗しました。(TITLE_MIDI)"),
            TEXT("IDirectMusicSegment8::SetParam"), MB_ICONERROR );
        return false;
    }
    hr = m_pTitleBGM->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(TITLE_MIDI)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pStage1BGM
    hFound = FindResource( NULL, TEXT("STAGE1_MIDI"), TEXT("MIDI") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pStage1BGM );
    if( FAILED(hr) ){
        m_pStage1BGM = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(STAGE1_MIDI)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pStage1BGM->SetParam( GUID_StandardMIDIFile, 0xFFFFFFFF, 0, 0, NULL );
    if( FAILED(hr) ){
        MessageBox( m_hWnd,
            TEXT("スタンダードMIDIファイルの初期化に失敗しました。(STAGE1_MIDI)"),
            TEXT("IDirectMusicSegment8::SetParam"), MB_ICONERROR );
        return false;
    }
    hr = m_pStage1BGM->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(STAGE1_MIDI)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pEndingBGM
    hFound = FindResource( NULL, TEXT("ENDING_MIDI"), TEXT("MIDI") );
    hResource = LoadResource( NULL, hFound );
    ObjDesc.pbMemData   = (LPBYTE)LockResource( hResource );
    ObjDesc.llMemLength = SizeofResource( NULL, hFound );
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void **)&m_pEndingBGM );
    if( FAILED(hr) ){
        m_pEndingBGM = NULL;
        MessageBox( m_hWnd, TEXT("IDirectMusicSegment8の取得に失敗しました。(ENDING_MIDI)"),
            TEXT("IDirectMusicLoader8::GetObject"), MB_ICONERROR );
        return false;
    }
    hr = m_pEndingBGM->SetParam( GUID_StandardMIDIFile, 0xFFFFFFFF, 0, 0, NULL );
    if( FAILED(hr) ){
        MessageBox( m_hWnd,
            TEXT("スタンダードMIDIファイルの初期化に失敗しました。(ENDING_MIDI)"),
            TEXT("IDirectMusicSegment8::SetParam"), MB_ICONERROR );
        return false;
    }
    hr = m_pEndingBGM->Download( m_pPerformance );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バンドのダウンロードに失敗しました。(ENDING_MIDI)"),
            TEXT("IDirectMusicSegment8::Download"), MB_ICONERROR );
        return false;
    }

    // m_pD3DMapVertex
    hr = m_pD3DDevice->CreateVertexBuffer( sizeof(FVFVERTEX)*(PieceLineNum+1)*2*2,
        0, FVFVERTEX::FORMAT, D3DPOOL_MANAGED, &m_pD3DMapVertex, NULL );
    if( FAILED(hr) ){
        m_pD3DMapVertex = NULL;
        MessageBox( m_hWnd, TEXT("頂点バッファの作成に失敗しました。(Map)"),
            TEXT("IDirect3DDevice9::CreateVertexBuffer"), MB_ICONERROR );
        return false;
    }
    hr = m_pD3DMapVertex->Lock( 0, 0, (void **)&pBuffer, 0 );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バッファのロックに失敗しました。(Map)"),
            TEXT("IDirect3DVertexBuffer9::Lock"), MB_ICONERROR );
        return false;
    }
    for( int i=0, n=0, nLine=0; i < (PieceLineNum+1); i++, n+=2, nLine++ ){
        pBuffer[n].x =
            (-(float)(PieceSize * PieceLineNum) / 2) + PieceSize * nLine;
        pBuffer[n].z = ((float)(PieceSize * PieceLineNum) / 2);
        pBuffer[n].y = pBuffer[n+1].y = 0.0f;
        pBuffer[n].color = pBuffer[n+1].color = MapLineColor;
        pBuffer[n+1].x = pBuffer[n].x; pBuffer[n+1].z = -pBuffer[n].z;
        pBuffer[n+(PieceLineNum+1)*2].x = (-(float)(PieceSize * PieceLineNum) / 2);
        pBuffer[n+(PieceLineNum+1)*2].z =
            ((float)(PieceSize * PieceLineNum) / 2) - PieceSize * nLine;
        pBuffer[n+(PieceLineNum+1)*2].color = MapLineColor;
        pBuffer[n+(PieceLineNum+1)*2].y = pBuffer[n+(PieceLineNum+1)*2+1].y = 0.0f;
        pBuffer[n+(PieceLineNum+1)*2+1].x = -pBuffer[n+(PieceLineNum+1)*2].x;
        pBuffer[n+(PieceLineNum+1)*2+1].z = pBuffer[n+(PieceLineNum+1)*2].z;
        pBuffer[n+(PieceLineNum+1)*2+1].color = MapLineColor;
    }
    m_pD3DMapVertex->Unlock();

    // m_pD3DPlaneVertex
    hr = m_pD3DDevice->CreateVertexBuffer( sizeof(FVFVERTEX)*4,
        0, FVFVERTEX::FORMAT, D3DPOOL_MANAGED, &m_pD3DPlaneVertex, NULL );
    if( FAILED(hr) ){
        m_pD3DPlaneVertex = NULL;
        MessageBox( m_hWnd, TEXT("頂点バッファの作成に失敗しました。(Plane)"),
            TEXT("IDirect3DDevice9::CreateVertexBuffer"), MB_ICONERROR );
        return false;
    }
    hr = m_pD3DPlaneVertex->Lock( 0, 0, (void **)&pBuffer, 0 );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バッファのロックに失敗しました。(Plane)"),
            TEXT("IDirect3DVertexBuffer9::Lock"), MB_ICONERROR );
        return false;
    }
    pBuffer[0].color = pBuffer[1].color = pBuffer[2].color = pBuffer[3].color = PlaneLineColor;
    pBuffer[0].x = 0.0f; pBuffer[0].y = (float)PieceSize/2; pBuffer[0].z = 0.0f;
    pBuffer[1].x = pBuffer[1].y = (float)PieceSize/2; pBuffer[1].z = -(float)PieceSize/2;
    pBuffer[2].x = pBuffer[2].z = -(float)PieceSize/2; pBuffer[2].y = (float)PieceSize/2;
    pBuffer[3] = pBuffer[0];
    m_pD3DMapVertex->Unlock();

    // m_pD3DBulletVertex
    hr = m_pD3DDevice->CreateVertexBuffer( sizeof(FVFVERTEX)*2,
        0, FVFVERTEX::FORMAT, D3DPOOL_MANAGED, &m_pD3DBulletVertex, NULL );
    if( FAILED(hr) ){
        m_pD3DBulletVertex = NULL;
        MessageBox( m_hWnd, TEXT("頂点バッファの作成に失敗しました。(Bullet)"),
            TEXT("IDirect3DDevice9::CreateVertexBuffer"), MB_ICONERROR );
        return false;
    }
    hr = m_pD3DBulletVertex->Lock( 0, 0, (void **)&pBuffer, 0 );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バッファのロックに失敗しました。(Bullet)"),
            TEXT("IDirect3DVertexBuffer9::Lock"), MB_ICONERROR );
        return false;
    }
    pBuffer[0].color = pBuffer[1].color = BulletLineColor;
    pBuffer[0].x = pBuffer[0].z = 0.0f; pBuffer[0].y = (float)PieceSize/2;
    pBuffer[1].x = 0; pBuffer[1].y = pBuffer[1].z = (float)PieceSize/2;
    m_pD3DBulletVertex->Unlock();

    // m_pD3DEnemy1Vertex
    hr = m_pD3DDevice->CreateVertexBuffer( sizeof(FVFVERTEX)*8,
        0, FVFVERTEX::FORMAT, D3DPOOL_MANAGED, &m_pD3DEnemy1Vertex, NULL );
    if( FAILED(hr) ){
        m_pD3DEnemy1Vertex = NULL;
        MessageBox( m_hWnd, TEXT("頂点バッファの作成に失敗しました。(Enemy1)"),
            TEXT("IDirect3DDevice9::CreateVertexBuffer"), MB_ICONERROR );
        return false;
    }
    hr = m_pD3DEnemy1Vertex->Lock( 0, 0, (void **)&pBuffer, 0 );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("バッファのロックに失敗しました。(Enemy1)"),
            TEXT("IDirect3DVertexBuffer9::Lock"), MB_ICONERROR );
        return false;
    }
    for( int j=0; j < 8; j++ ){
        pBuffer[j].color = Enemy1LineColor;
    }
    pBuffer[0].x = -(float)PieceSize/4; pBuffer[0].y = (float)PieceSize/2;   pBuffer[0].z = 0.0f;
    pBuffer[1].x = -(float)PieceSize/4; pBuffer[1].y = (float)PieceSize/2;
        pBuffer[1].z = (float)PieceSize/2;
    pBuffer[2].x = (float)PieceSize/4;  pBuffer[2].y = (float)PieceSize/2;   pBuffer[2].z = 0.0f;
    pBuffer[3].x = (float)PieceSize/4;  pBuffer[3].y = (float)PieceSize/2;
        pBuffer[3].z = (float)PieceSize/2;
    pBuffer[4].x = -(float)PieceSize/4; pBuffer[4].y = 0.0f; pBuffer[4].z = 0.0f;
    pBuffer[5].x = -(float)PieceSize/4; pBuffer[5].y = 0.0f; pBuffer[5].z = (float)PieceSize/2;
    pBuffer[6].x = (float)PieceSize/4;  pBuffer[6].y = 0.0f; pBuffer[6].z = 0.0f;
    pBuffer[7].x = (float)PieceSize/4;  pBuffer[7].y = 0.0f; pBuffer[7].z = (float)PieceSize/2;
    m_pD3DEnemy1Vertex->Unlock();
    // m_pD3DEnemy1IBuffer
    hr = m_pD3DDevice->CreateIndexBuffer( 16 * 2, 0,
        D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pD3DEnemy1IBuffer, NULL );
    if( FAILED(hr) ){
        m_pD3DEnemy1IBuffer = NULL;
        MessageBox( m_hWnd, TEXT("インデックスバッファの作成に失敗しました。(Enemy1)"),
            TEXT("IDirect3DDevice9::CreateIndexBuffer"), MB_ICONERROR );
        return false;
    }
    hr = m_pD3DEnemy1IBuffer->Lock( 0, 0, (void **)&pIndex, 0 );
    if( FAILED(hr) ){
        MessageBox( m_hWnd, TEXT("インデックスバッファのロックに失敗しました。(Enemy1)"),
            TEXT("IDirect3DIndexBuffer9::Lock"), MB_ICONERROR );
        return false;
    }
    pIndex[0] =  0; pIndex[1] =  1; pIndex[2] =  3; pIndex[3] =  2; pIndex[4] =  0;
    pIndex[5] =  4; pIndex[6] =  5; pIndex[7] =  1; pIndex[8] =  5; pIndex[9] =  7;
    pIndex[10] = 3; pIndex[11] = 7; pIndex[12] = 6; pIndex[13] = 2; pIndex[14] = 6;
    pIndex[15] = 4;
    m_pD3DEnemy1IBuffer->Unlock();

    return true;
}

// その他オブジェクトの開放
void CDX::CleanupObject()
{
    RELEASE( m_pD3DEnemy1IBuffer );
    RELEASE( m_pD3DEnemy1Vertex );
    RELEASE( m_pD3DBulletVertex );
    RELEASE( m_pD3DPlaneVertex );
    RELEASE( m_pD3DMapVertex );
    RELEASE( m_pEndingBGM );
    RELEASE( m_pStage1BGM );
    RELEASE( m_pTitleBGM );
    RELEASE( m_pPauseSound );
    RELEASE( m_pEndingSound );
    RELEASE( m_pKilledE1Sound );
    RELEASE( m_pShootSound );
    RELEASE( m_pStartSound );
    RELEASE( m_pD3DXTitleFont );
    RELEASE( m_pD3DXSysFont );
    RELEASE( m_p3DBuffer );
    RELEASE( m_pListener );
    RELEASE( m_p3DAudioPath );
}

// DirectX Graphicsに管理されないオブジェクトのクリーンアップ
void CDX::CleanupD3DObject()
{
    m_pD3DXSysFont->OnLostDevice();
    m_pD3DXTitleFont->OnLostDevice();
}

// DirectX Graphicsに管理されないオブジェクトの初期化
void CDX::InitD3DObject()
{
    m_pD3DXSysFont->OnResetDevice();
    m_pD3DXTitleFont->OnResetDevice();
}


// public section //

// コンストラクタ
CDX::CDX( HWND hWnd )
: m_bInit(false), m_bDeviceLost(false), m_bSetViewport(false), m_bMusicCOM(false),
    m_pD3D(NULL), m_pD3DDevice(NULL), m_pDIDevice(NULL), m_pDInput(NULL),
    m_pLoader(NULL), m_pPerformance(NULL), m_hWnd(hWnd),
    m_p3DAudioPath(NULL), m_pListener(NULL), m_p3DBuffer(NULL),
    m_pD3DXSysFont(NULL), m_pD3DXTitleFont(NULL), m_pStartSound(NULL), m_pStage1BGM(NULL),
    m_pTitleBGM(NULL), m_pD3DMapVertex(NULL), m_pD3DPlaneVertex(NULL),
    m_pD3DBulletVertex(NULL), m_pShootSound(NULL),m_pD3DEnemy1Vertex(NULL),
    m_pD3DEnemy1IBuffer(NULL), m_pKilledE1Sound(NULL), m_pEndingBGM(NULL),
    m_pEndingSound(NULL), m_pPauseSound(NULL)
{
    m_hInstance = (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE );

    if( InitDXGraphics() )
        if( InitDInput() )
            if( InitDMusic() )
                if( InitObject() )
                    m_bInit = true;
}

// デストラクタ
CDX::~CDX()
{
    CleanupObject();
    CleanupDMusic();
    CleanupDInput();
    CleanupDXGraphics();
}

// ビューポートのセット
void CDX::SetViewport()
{
    D3DVIEWPORT9 vp;

    vp.X        = 0;
    vp.Y        = 0;
    vp.Width    = m_D3DPP.BackBufferWidth;
    vp.Height   = m_D3DPP.BackBufferHeight;
    vp.MinZ     = 0.0f;
    vp.MaxZ     = 1.0f;
    if( FAILED(m_pD3DDevice->SetViewport(&vp)) ){
        m_bSetViewport = false;
        return;
    }
    m_bSetViewport = true;
}

// 消失したデバイスのリセット
void CDX::DeviceLost()
{
    HRESULT hr;

    hr = m_pD3DDevice->TestCooperativeLevel();
    if( FAILED(hr) ){
        if( hr != D3DERR_DEVICENOTRESET )
            return; // デバイスは消失している。(リセット不可)

        CleanupD3DObject();
        hr = m_pD3DDevice->Reset( &m_D3DPP );
        if( FAILED(hr) ){
            if( hr == D3DERR_DEVICELOST ){
                return;
            }
            MessageBox( m_hWnd, TEXT("デバイスのリセットに失敗しました。"),
                TEXT("IDirect3DDevice9::Reset"), MB_ICONERROR );
            DestroyWindow( m_hWnd );
            return;
        }
        InitD3DObject();
        SetState();
    }
    m_bDeviceLost = false;
}

// キーボードの状態を取得
bool CDX::GetKeyState()
{
    HRESULT hr;

    hr = m_pDIDevice->GetDeviceState( 256, m_pKeyState );
    if( FAILED(hr) ){
        if( SUCCEEDED(m_pDIDevice->Acquire()) ){
            return GetKeyState();
        }
        return false;
    }

    return true;
}

// GetKeyStateで取得した情報の検証
bool CDX::CheckKeyState( DWORD dwKeyState )
{
    return (m_pKeyState[dwKeyState] & 0x80)?true:false;
}

// マスターボリュームの設定
bool CDX::SetMasterVolume( long lVolume )
{
    HRESULT hr;

    hr = m_pPerformance->SetGlobalParam(
        GUID_PerfMasterVolume, (void *)&lVolume, sizeof(lVolume) );
    if( FAILED(hr) )
        return false;

    return true;
}

// BGMを再生(ループ有り)
bool CDX::PlayBGM( IDirectMusicSegment8 *pSource, IUnknown *pAudioPath )
{
    HRESULT hr;
    
    hr = pSource->SetRepeats( DMUS_SEG_REPEAT_INFINITE );
    if( FAILED(hr) )
        return false;

    hr = m_pPerformance->PlaySegmentEx( pSource, NULL, NULL, 0, 0, NULL, NULL, pAudioPath );
    if( FAILED(hr) )
        return false;

    return true;
}

// pSource1を再生したあとpSource2を再生
bool CDX::PlaySoundWithBGM( IDirectMusicSegment8 *pSource1,
    IDirectMusicSegment8 *pSource2, IUnknown *pAudioPath )
{
    HRESULT hr;
    
    hr = pSource2->SetRepeats( DMUS_SEG_REPEAT_INFINITE );
    if( FAILED(hr) )
        return false;

    hr = m_pPerformance->PlaySegmentEx( pSource1, NULL, NULL, 0, 0, NULL, NULL, pAudioPath );
    if( FAILED(hr) )
        return false;

    hr = m_pPerformance->PlaySegmentEx( pSource2,
        NULL, NULL, DMUS_SEGF_SEGMENTEND, 0, NULL, NULL, pAudioPath );
    if( FAILED(hr) )
        return false;

    return true;
}

// セカンダリバッファで再生(ループ無し)
bool CDX::PlaySecondary( IDirectMusicSegment8 *pSource, IUnknown *pAudioPath )
{
    HRESULT hr;

    hr = pSource->SetRepeats( 0 );
    if( FAILED(hr) )
        return false;

    hr = m_pPerformance->PlaySegmentEx( pSource, NULL, NULL,
        DMUS_SEGF_SECONDARY, 0, NULL, NULL, pAudioPath );
    if( FAILED(hr) )
        return false;

    return true;
}

// 再生停止
bool CDX::StopSound( IUnknown *pObjectToStop )
{
    if( FAILED(m_pPerformance->StopEx(pObjectToStop, 0, 0)) )
        return false;

    return true;
}

// 全再生停止
bool CDX::StopSoundALL()
{
    if( FAILED(m_pPerformance->Stop(NULL, NULL, 0, 0)) )
        return false;

    return true;
}
