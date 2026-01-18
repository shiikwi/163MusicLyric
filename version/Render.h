#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Render
{
	class LyricRender
	{
	public:
		void Init(HWND hwnd);
		void Draw();
		void RecreateRT(HWND hwnd);
	private:
		ComPtr<ID2D1Factory> m_pD2DFactory;
		ComPtr<ID2D1HwndRenderTarget> m_pRT;
		ComPtr<IDWriteFactory> m_pDWFactory;
	};
}