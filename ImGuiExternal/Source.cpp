#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <psapi.h>
#include <dwmapi.h>
#include <d3d9.h>
#include "Functions.h"
#include "Overlay.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

LPCSTR TargetProcess = "D3D9Test.exe";
bool ShowMenu = true;
bool CreateConsole = true;

struct CurrentProcess {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	int WindowLeft;
	int WindowRight;
	int WindowTop;
	int WindowBottom;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}Process;

struct OverlayWindow {
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}Overlay;

struct DirectX9Interface {
	IDirect3D9Ex* IDirect3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParameters = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}DirectX9;

void InputHandler() {
	for (int i = 0; i < 5; i++) {
		ImGui::GetIO().MouseDown[i] = false;
	}

	int Button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) {
		Button = 0;
	}

	if (Button != -1) {
		ImGui::GetIO().MouseDown[Button] = true;
	}
}

void Draw() {
	char FpsInfo[64];
	sprintf(FpsInfo, "Overlay FPS: %0.f", ImGui::GetIO().Framerate);
	RGBA White = { 255,255,255,255 };
	DrawStrokeText(30, 44, &White, FpsInfo);
}

void Render() {
	if (GetAsyncKeyState(VK_INSERT) & 1) ShowMenu = !ShowMenu;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	Draw();
	ImGui::GetIO().MouseDrawCursor = ShowMenu;

	if (ShowMenu == true) {
		InputHandler();
		ImGui::ShowDemoWindow();
		SetWindowLong(Overlay.Hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		UpdateWindow(Overlay.Hwnd);
		SetFocus(Overlay.Hwnd);
	}
	else {
		SetWindowLong(Overlay.Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
		UpdateWindow(Overlay.Hwnd);
	}
	ImGui::EndFrame();

	DirectX9.pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (DirectX9.pDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DirectX9.pDevice->EndScene();
	}

	HRESULT result = DirectX9.pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9.pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		DirectX9.pDevice->Reset(&DirectX9.pParameters);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void MainLoop() {
	static RECT OldRect;
	ZeroMemory(&DirectX9.Message, sizeof(MSG));
	while (DirectX9.Message.message != WM_QUIT) {
		if (PeekMessage(&DirectX9.Message, Overlay.Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&DirectX9.Message);
			DispatchMessage(&DirectX9.Message);
		}
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == Process.Hwnd) {
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(Overlay.Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(Process.Hwnd, &TempRect);
		ClientToScreen(Process.Hwnd, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = Process.Hwnd;

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
			OldRect = TempRect;
			Process.WindowWidth = TempRect.right;
			Process.WindowHeight = TempRect.bottom;
			DirectX9.pParameters.BackBufferWidth = Process.WindowWidth;
			DirectX9.pParameters.BackBufferHeight = Process.WindowHeight;
			SetWindowPos(Overlay.Hwnd, (HWND)0, TempPoint.x, TempPoint.y, Process.WindowWidth, Process.WindowHeight, SWP_NOREDRAW);
			DirectX9.pDevice->Reset(&DirectX9.pParameters);
		}
		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (DirectX9.pDevice != NULL) {
		DirectX9.pDevice->EndScene();
		DirectX9.pDevice->Release();
	}
	if (DirectX9.IDirect3D9 != NULL) {
		DirectX9.IDirect3D9->Release();
	}
	DestroyWindow(Overlay.Hwnd);
	UnregisterClass(Overlay.WindowClass.lpszClassName, Overlay.WindowClass.hInstance);
}

bool DirectXInit() {
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9.IDirect3D9))) {
		return false;
	}

	D3DPRESENT_PARAMETERS Params = { 0 };
	Params.Windowed = TRUE;
	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	Params.hDeviceWindow = Overlay.Hwnd;
	Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	Params.BackBufferWidth = Process.WindowWidth;
	Params.BackBufferHeight = Process.WindowHeight;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.EnableAutoDepthStencil = TRUE;
	Params.AutoDepthStencilFormat = D3DFMT_D16;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if (FAILED(DirectX9.IDirect3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Overlay.Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9.pDevice))) {
		DirectX9.IDirect3D9->Release();
		return false;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplWin32_Init(Overlay.Hwnd);
	ImGui_ImplDX9_Init(DirectX9.pDevice);
	DirectX9.IDirect3D9->Release();
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message) {
	case WM_DESTROY:
		if (DirectX9.pDevice != NULL) {
			DirectX9.pDevice->EndScene();
			DirectX9.pDevice->Release();
		}
		if (DirectX9.IDirect3D9 != NULL) {
			DirectX9.IDirect3D9->Release();
		}
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (DirectX9.pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			DirectX9.pParameters.BackBufferWidth = LOWORD(lParam);
			DirectX9.pParameters.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = DirectX9.pDevice->Reset(&DirectX9.pParameters);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void SetupWindow() {
	Overlay.WindowClass = {
		sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, Overlay.Name, LoadIcon(nullptr, IDI_APPLICATION)
	};

	RegisterClassEx(&Overlay.WindowClass);
	if (Process.Hwnd) {
		static RECT TempRect = { NULL };
		static POINT TempPoint;
		GetClientRect(Process.Hwnd, &TempRect);
		ClientToScreen(Process.Hwnd, &TempPoint);
		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		Process.WindowWidth = TempRect.right;
		Process.WindowHeight = TempRect.bottom;
	}

	Overlay.Hwnd = CreateWindowEx(NULL, Overlay.Name, Overlay.Name, WS_POPUP | WS_VISIBLE, Process.WindowLeft, Process.WindowTop, Process.WindowWidth, Process.WindowHeight, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(Overlay.Hwnd, &DirectX9.Margin);
	SetWindowLong(Overlay.Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	ShowWindow(Overlay.Hwnd, SW_SHOW);
	UpdateWindow(Overlay.Hwnd);
}

DWORD WINAPI ProcessCheck(LPVOID lpParameter) {
	while (true) {
		if (Process.Hwnd != NULL) {
			if (GetProcessId(TargetProcess) == 0) {
				exit(0);
			}
		}
	}
}

int main() {
	if (CreateConsole == false)
		ShowWindow(GetConsoleWindow(), SW_HIDE);

	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetProcessId(TargetProcess) == ForegroundWindowProcessID) {
			Process.ID = GetCurrentProcessId();
			Process.Handle = GetCurrentProcess();
			Process.Hwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(Process.Hwnd, &TempRect);
			Process.WindowWidth = TempRect.right - TempRect.left;
			Process.WindowHeight = TempRect.bottom - TempRect.top;
			Process.WindowLeft = TempRect.left;
			Process.WindowRight = TempRect.right;
			Process.WindowTop = TempRect.top;
			Process.WindowBottom = TempRect.bottom;

			char TempTitle[MAX_PATH];
			GetWindowText(Process.Hwnd, TempTitle, sizeof(TempTitle));
			Process.Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(Process.Hwnd, TempClassName, sizeof(TempClassName));
			Process.ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process.Handle, NULL, TempPath, sizeof(TempPath));
			Process.Path = TempPath;

			WindowFocus = true;
		}
	}

	Overlay.Name = RandomString(10).c_str();
	SetupWindow();
	DirectXInit();
	CreateThread(0, 0, ProcessCheck, 0, 0, 0);
	while (TRUE) {
		MainLoop();
	}
}