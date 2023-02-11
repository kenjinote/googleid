#pragma comment(lib,"imm32")
#pragma comment(lib,"crypt32")
#pragma comment(lib,"libeay32")
#pragma comment(lib,"ssleay32")
#pragma comment(lib,"ws2_32")

#define UNICODE
#include<winsock2.h>
#include"openssl/ssl.h"
#include<windows.h>

TCHAR szClassName[]=TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HWND hEdit;
	switch(msg)
	{
	case WM_CREATE:
		hEdit=CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"),0,WS_CHILD|WS_VISIBLE|WS_TABSTOP,10,10,256,30,hWnd,0,((LPCREATESTRUCT)lParam)->hInstance,0);
		CreateWindow(TEXT("BUTTON"),TEXT("確認"),WS_CHILD|WS_VISIBLE|WS_TABSTOP,10,50,256,30,hWnd,(HMENU)IDOK,((LPCREATESTRUCT)lParam)->hInstance,0);
		ImmAssociateContext(hEdit,0);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDOK&&GetWindowTextLength(hEdit))
		{
			EnableWindow(hWnd,0);
			LPSTR lpszResponse;
			WSADATA wsa;
			WSAStartup(MAKEWORD(1,1),&wsa);
			SOCKET s=socket(AF_INET,SOCK_STREAM,0);
			struct sockaddr_in addr;
			struct hostent *host;
			addr.sin_family=AF_INET;
			addr.sin_port=htons(443);
			host=gethostbyname("www.google.com");
			addr.sin_addr=*((struct in_addr*)*host->h_addr_list);
			connect(s,(struct sockaddr*)&addr,sizeof(addr));
			
			SSL_library_init();
			SSL_load_error_strings();
			SSL_CTX* ctx=SSL_CTX_new(SSLv23_client_method());
			SSL* ssl=SSL_new(ctx);
			SSL_set_fd(ssl,(DWORD)s);
			SSL_connect(ssl);
			
			CHAR szAccount[1024];
			CHAR szRequest[1024];
			CHAR szBuffer[1024];
			GetWindowTextA(hEdit,szAccount,1024);
			wsprintfA(szRequest,"GET /accounts/CheckAvailability?Email=%s\r\n",szAccount);
			SSL_write(ssl,szRequest,lstrlenA(szRequest));
			lpszResponse=(LPSTR)GlobalAlloc(GMEM_FIXED,1);
			LPSTR lpszTemp;
			DWORD dwBufferSize;
			lpszResponse[0]=0;
			while(1)
			{
				DWORD n=SSL_read(ssl,szBuffer,sizeof(szBuffer)-1);
				if(n==0)break;
				szBuffer[n]=0;
				dwBufferSize=lstrlenA(lpszResponse)+lstrlenA(szBuffer);
				lpszTemp=(LPSTR)GlobalAlloc(GMEM_FIXED,dwBufferSize+1);
				lstrcpyA(lpszTemp,lpszResponse);
				GlobalFree(lpszResponse);
				lstrcatA(lpszTemp,szBuffer);
				lpszResponse=lpszTemp;
			}
			SSL_shutdown(ssl);
			SSL_free(ssl);
			SSL_CTX_free(ctx);
			shutdown(s,SD_BOTH);
			closesocket(s);
			WSACleanup();
			if(strstr(lpszTemp,"is available"))
				MessageBox(hWnd,TEXT("取得可能です。"),TEXT("確認"),0);
			else if(strstr(lpszTemp,"is not available"))
				MessageBox(hWnd,TEXT("そのユーザー名はもう使われています。"),TEXT("確認"),0);
			else 
				MessageBox(hWnd,TEXT("失敗しました。"),0,0);
			GlobalFree(lpszResponse);
			EnableWindow(hWnd,1);
			SetFocus(hEdit);
			SendMessage(hEdit,EM_SETSEL,0,-1);
			return 1;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefDlgProc(hWnd,msg,wParam,lParam));
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPreInst,LPSTR pCmdLine,int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass={0,WndProc,0,DLGWINDOWEXTRA,hInstance,0,LoadCursor(0,IDC_ARROW),0,0,szClassName};
	RegisterClass(&wndclass);
	HWND hWnd=CreateWindow(szClassName,TEXT("指定したGMailのユーザー名が登録可能か調べる"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,0,CW_USEDEFAULT,0,0,0,hInstance,0);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,0,0,0))
	{
		if(!IsDialogMessage(hWnd,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return(msg.wParam);
}
