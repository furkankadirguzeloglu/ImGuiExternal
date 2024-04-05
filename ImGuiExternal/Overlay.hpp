#pragma once
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui_impl_win32.h>

void inputHandler();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#define clearVariable(x) if (x) { x->Release(); x = nullptr; delete x;}

std::string generateRandomString(int length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string randomString;
    std::srand(std::time(0));
    for (int i = 0; i < length; ++i) {
        randomString += characters[std::rand() % characters.length()];
    }
    return randomString;
}

int generateRandomInt(int min, int max) {
    std::srand(std::time(0));
    return min + std::rand() % (max - min + 1);
}

DWORD getProcessID(std::string processName) {
    PROCESSENTRY32 processInfo;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    processInfo.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &processInfo)) {
        do {
            if (!lstrcmpi(processInfo.szExeFile, processName.c_str())) {
                CloseHandle(hSnapshot);
                return processInfo.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &processInfo));
    }
    CloseHandle(hSnapshot);
    return 0;
}

typedef struct
{
	DWORD R;
	DWORD G;
	DWORD B;
	DWORD A;
}RGBA;

std::string stringToUTF8(const std::string& str) {
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t* pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);
	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	std::string retStr(pBuf);
	delete[]pwBuf;
	delete[]pBuf;
	pwBuf = NULL;
	pBuf = NULL;
	return retStr;
}

void drawStrokeText(int x, int y, RGBA* color, const char* str) {
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = stringToUTF8(utf_8_1);
	ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetForegroundDrawList()->AddText(ImVec2(x - 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetForegroundDrawList()->AddText(ImVec2(x + 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
	ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), utf_8_2.c_str());
}

void drawNewText(int x, int y, RGBA* color, const char* str) {
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = stringToUTF8(utf_8_1);
	ImGui::GetForegroundDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), utf_8_2.c_str());
}

void drawRect(int x, int y, int w, int h, RGBA* color, int thickness) {
	ImGui::GetForegroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), 0, 0, thickness);
}

void drawFilledRect(int x, int y, int w, int h, RGBA* color) {
	ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), 0, 0);
}

void drawCircleFilled(int x, int y, int radius, RGBA* color) {
	ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
}

void drawCircle(int x, int y, int radius, RGBA* color, int segments) {
	ImGui::GetForegroundDrawList()->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), segments);
}

void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, RGBA* color, float thickne) {
	ImGui::GetForegroundDrawList()->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), thickne);
}

void drawTriangleFilled(int x1, int y1, int x2, int y2, int x3, int y3, RGBA* color) {
	ImGui::GetForegroundDrawList()->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
}

void drawLine(int x1, int y1, int x2, int y2, RGBA* color, int thickness) {
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), thickness);
}

void drawCornerBox(int x, int y, int w, int h, int borderPx, RGBA* color) {
	drawFilledRect(x + borderPx, y, w / 3, borderPx, color);
	drawFilledRect(x + w - w / 3 + borderPx, y, w / 3, borderPx, color);
	drawFilledRect(x, y, borderPx, h / 3, color);
	drawFilledRect(x, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color);
	drawFilledRect(x + borderPx, y + h + borderPx, w / 3, borderPx, color);
	drawFilledRect(x + w - w / 3 + borderPx, y + h + borderPx, w / 3, borderPx, color);
	drawFilledRect(x + w + borderPx, y, borderPx, h / 3, color);
	drawFilledRect(x + w + borderPx, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color);
}