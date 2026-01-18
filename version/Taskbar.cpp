#include "Taskbar.h"
#include "Config.h"
#include "Utils.h"

namespace Taskbar
{
	void TaskbarWindow::Create()
	{
		WNDCLASSEXW wc = { sizeof(wc), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"TaskbarLyricWnd", NULL };
		RegisterClassExW(&wc);

		HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
		m_hwnd = CreateWindowEx(
			WS_EX_LAYERED | WS_EX_TRANSPARENT | (Config::g_Config.AlwaysTop ? WS_EX_TOPMOST : 0) | WS_EX_TOOLWINDOW,
			L"TaskbarLyricWnd", L"",
			WS_POPUP,
			0, 0, 1, 1,
			hTray, NULL, GetModuleHandle(NULL), NULL
		);

		if (!m_hwnd) return;

		m_render.Init(m_hwnd);
		SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
		Resize();
		ShowWindow(m_hwnd, SW_SHOW);
		SetTimer(m_hwnd, 1, 1000, NULL);
	}

	static BOOL CALLBACK FindTaskList(HWND hwnd, LPARAM lParam)
	{
		wchar_t className[256];
		if (GetClassNameW(hwnd, className, 256) && wcscmp(className, L"MSTaskListWClass") == 0)
		{
			*(HWND*)lParam = hwnd;
			return false;
		}
		return true;
	}

	void TaskbarWindow::Resize()
	{
		HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
		HWND hTaskList = NULL;
		EnumChildWindows(hTray, FindTaskList, (LPARAM)&hTaskList);

		RECT trayRect, inputRect;
		GetWindowRect(hTray, &trayRect);

		LONG x = trayRect.left + Config::g_Config.XLeftMargin;
		LONG y = trayRect.top;
		LONG h = trayRect.bottom - trayRect.top;
		LONG w = 400;

		if (hTaskList)
		{
			GetWindowRect(hTaskList, &inputRect);
			w = inputRect.left - x - Config::g_Config.XRightMargin;
		}
		if (w < 10) w = 10;

		static RECT lastRect;
		auto hOrder = Config::g_Config.AlwaysTop ? HWND_TOPMOST : HWND_NOTOPMOST;
		if (x != lastRect.left || w != (lastRect.right - lastRect.left))
		{
			SetWindowPos(m_hwnd, hOrder, x, y, w, h, SWP_NOACTIVATE);
			m_render.RecreateRT(m_hwnd);
			Utils::Logger::Log("Window Pos: x={}, y={}, w={}, h={}", x, y, w, h);
			lastRect = { x, y, x + w, y + h };
		}
		else
		{
			SetWindowPos(m_hwnd, hOrder, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
	LRESULT TaskbarWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_PAINT:
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			Instance().m_render.Draw();
			EndPaint(hwnd, &ps);
			return 0;
		case WM_TIMER:
			if (wp == 1)
			{
				Instance().Resize();
				InvalidateRect(hwnd, NULL, false);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProcW(hwnd, msg, wp, lp);
		}
		return 0;
	}
}