#include "Taskbar.h"
#include "Config.h"
#include "Utils.h"

namespace Taskbar
{
	void TaskbarWindow::Create()
	{
		WNDCLASSEXW wc = { sizeof(wc), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"TaskbarLyricWnd", NULL };
		RegisterClassExW(&wc);

		m_hwnd = CreateWindowEx(
			WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
			L"TaskbarLyricWnd", L"",
			WS_POPUP,
			0, 0, 1, 1,
			NULL, NULL, GetModuleHandle(NULL), NULL
		);

		if (!m_hwnd) return;

		m_render.Init(m_hwnd);
		SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
		Resize();
		ShowWindow(m_hwnd, SW_SHOW);
		SetTimer(m_hwnd, 1, 1000, NULL);
	}

	void TaskbarWindow::Resize()
	{
		HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
		HWND hInputSite = FindWindowExW(hTray, NULL, L"Windows.UI.Input.InputSite.WindowClass", NULL);

		RECT trayRect, inputRect;
		GetWindowRect(hTray, &trayRect);

		int x = trayRect.left + (int)Config::g_Config.XLeftMargin;
		int y = trayRect.top;
		int h = trayRect.bottom - trayRect.top;
		int w = 400;

		if (hInputSite)
		{
			GetWindowRect(hInputSite, &inputRect);
			w = inputRect.left - x - (int)Config::g_Config.XRightMargin;
		}
		if (w < 10) w = 10;

		static RECT lastRect;
		if (x != lastRect.left || w != (lastRect.right - lastRect.left))
		{
			SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE);
			m_render.RecreateRT(m_hwnd);
			Utils::Logger::Log("Window Pos: x={}, y={}, w={}, h={}", x, y, w, h);
			lastRect = { x, y, x + w, y + h };
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