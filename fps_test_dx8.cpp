#include "framework.h"
#include "fps_test_dx8.h"

#include <d3d8.h>
#include <mmsystem.h>
#include <stdio.h>

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

HWND hWnd = NULL;                               // ウインドウハンドル

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#pragma comment(lib,"d3d8.lib")
#pragma comment(lib,"winmm.lib")

void FpsTestInit(void);
void FpsTestTick(void);

LPDIRECT3D8 G_D3D_PTR = NULL;
LPDIRECT3DDEVICE8 G_D3D_DEVICE_PTR = NULL;
DWORD prev_time;

void FpsTestInit(void)
{
    // 念のため
    if (hWnd == NULL)
        throw _T("FpsTestInit: NULL: hWnd");

    FILE* fp;
    AllocConsole();
    freopen_s(&fp, "CONOUT$", "w", stdout);

    G_D3D_PTR = Direct3DCreate8(D3D_SDK_VERSION);
    if (G_D3D_PTR == NULL)
        throw _T("FpsTestInit: FAIL: Direct3DCreate8");

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.BackBufferWidth = 640;
    d3dpp.BackBufferHeight = 480;

    D3DDISPLAYMODE mode;
    G_D3D_PTR->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
    d3dpp.BackBufferFormat = mode.Format;

    d3dpp.BackBufferCount = 0;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;  // 0
    d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;  // 2
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Windowed = FALSE;  // 0  // フルスクリーン
    d3dpp.EnableAutoDepthStencil = TRUE;  // 1
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;  // 80
    d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;  // 1
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;  // 0
    d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;  // 1

    if (FAILED(G_D3D_PTR->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &G_D3D_DEVICE_PTR)))
        throw _T("FpsTestInit: FAIL: LPDIRECT3D8->CreateDevice");

    timeBeginPeriod(1);
    prev_time = timeGetTime();
    timeEndPeriod(1);

    OutputDebugString(_T("init success\n"));
}

void FpsTestTick(void)
{
    static int frame_count = 0;

    DWORD now_time;

    // プロローグ
    G_D3D_DEVICE_PTR->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0xff, 0xff, 0xff), 1.0f, 0);
    G_D3D_DEVICE_PTR->BeginScene();

    // 処理
    frame_count++;
    timeBeginPeriod(1);
    now_time = timeGetTime();
    timeEndPeriod(1);
    if (now_time - prev_time >= 1000/* 1s */)
    {
        double fps;
        fps = (double)frame_count * ((double)(now_time - prev_time) / (double)1000.0);
        wprintf(_T("%lf FPS\n"), fps);

        prev_time = now_time;
        frame_count = 0;
    }

    // エピローグ
    G_D3D_DEVICE_PTR->EndScene();
    G_D3D_DEVICE_PTR->Present(NULL, NULL, NULL, NULL);
}

int APIENTRY _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FPSTESTDX8, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FPSTESTDX8));

    MSG msg;

    FpsTestInit();

    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
            FpsTestTick();
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FPSTESTDX8));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FPSTESTDX8);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
