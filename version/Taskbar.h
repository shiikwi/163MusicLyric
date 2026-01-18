#pragma once
#include <Windows.h>
#include "Render.h"

namespace Taskbar
{
	class TaskbarWindow
	{
	public:
		static TaskbarWindow& Instance()
		{
			static TaskbarWindow instance;
			return instance;
		}

		void Create();
		void Resize();
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	private:
		TaskbarWindow(const TaskbarWindow&) = delete;
		TaskbarWindow& operator=(const TaskbarWindow&) = delete;
		TaskbarWindow() = default;
		~TaskbarWindow() = default;

		HWND m_hwnd = nullptr;
		Render::LyricRender m_render;
	};
}

