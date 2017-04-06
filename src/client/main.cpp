#include "client.h"
#include <QCoreApplication>

static QCoreApplication* app;

BOOL CtrlHandler( DWORD fdwCtrlType )
{
  switch( fdwCtrlType )
  {
    case CTRL_C_EVENT:
      app->quit();
      return( TRUE );

    case CTRL_CLOSE_EVENT:
      app->quit();
      return( TRUE );

    case CTRL_BREAK_EVENT:
      app->quit();
      return TRUE;

    case CTRL_LOGOFF_EVENT:
      return FALSE;

    case CTRL_SHUTDOWN_EVENT:
      return FALSE;

    default:
      return FALSE;
  }
}

/*
LRESULT CALLBACK MainWndProc ( HWND hwnd , UINT msg , WPARAM wParam, LPARAM lParam )
{
    qDebug() << msg;
    switch(msg)
    {
        case WM_QUERYENDSESSION:
            MessageBox(NULL, L"Received WM_QUERYENDSESSION", L"MainWndProc",MB_OK);
            return TRUE;

        case WM_ENDSESSION:
            MessageBox(NULL, L"Received WM_ENDSESSION", L"MainWndProc",MB_OK);
            if (wParam)
             MessageBox(hwnd, L"ConsoleWaWindow", L"WM_ENDSESSION at any TIME!!", MB_OK);
            else
                MessageBox(hwnd, L"ConsoleWaWindow", L"WM_ENDSESSION aNO!!", MB_OK);
            return TRUE;

        case WM_DESTROY:
            {
            PostQuitMessage(0);
            }
            break;

        case WM_CLOSE:
            break;
            default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
            break;
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        switch (message)
        {
                case WM_DISPLAYCHANGE:
                        qDebug() << "Display Change";
                        break;
                case WM_CLOSE:
                case WM_QUIT:
                case WM_QUERYENDSESSION:
                        DestroyWindow(hWnd);
                        PostQuitMessage(0);
                        break;
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
}

void CreateInvisibleWindow()
{

    //HWND hwnd;
    //WNDCLASS wc={0};
    //wc.lpfnWndProc=(WNDPROC)WndProc;
    //wc.hInstance=GetModuleHandle(NULL);
    //wc.hIcon=LoadIcon(GetModuleHandle(NULL), L"TestWClass");
    //wc.lpszClassName = L"TestWClass";
    //RegisterClass(&wc);

    //hwnd=CreateWindowEx(0, L"TestWClass", L"TestWClass",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,(HWND) NULL, (HMENU) NULL, GetModuleHandle(NULL), (LPVOID) NULL);


    HWND hwnd;
    LPCTSTR szClassName = L"BackgroundWindow";
    WNDCLASSEX wc;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = 0;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClassName;
    wc.hIconSm       = NULL;

    RegisterClassEx(&wc);

    hwnd = CreateWindowEx(WS_EX_NOACTIVATE, szClassName, 0, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, HWND_DESKTOP, NULL, hInstance, NULL);

    //ShowWindow(hwnd, 5);

    if(!hwnd)
        printf("FAILED to create window!!!  %d\n",GetLastError());
    else
        printf("success\n");
}

DWORD WINAPI RunInvisibleWindowThread(LPVOID lpParam)
{
    MSG msg;
    CreateInvisibleWindow();
    printf(" Thread and window created\n");
    while (GetMessage(&msg,(HWND) NULL , 0 , 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

*/

int main(int argc, char *argv[])
{

    app = new QCoreApplication(argc, argv);
    Client* client1;

    //DWORD tid;
    //HANDLE hInvisiblethread=CreateThread(NULL, 0, RunInvisibleWindowThread, NULL, 0, &tid);
    //CloseHandle(hInvisiblethread);

    //Set console ctrl handler
    if( ! SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) )
        qDebug() << "Handle installation error.";

    //Get command line arguments
    if (argc <= 1)
    {
        client1 = new Client(app, QDir::currentPath(), "127.0.0.1", 12345);
    }
    else if (argc == 3)
    {
        //Set ip + port
        client1 = new Client(app, QDir::currentPath(), argv[1], QString(argv[2]).toInt());
    }
    else if (argc > 3)
    {
        //Set path + ip + port
        client1 = new Client(app, argv[1], argv[2], QString(argv[3]).toInt());
    }

    QObject::connect(app, &QCoreApplication::aboutToQuit, client1, &Client::deleteLater);

    return app->exec();
}
