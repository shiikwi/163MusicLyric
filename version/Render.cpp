#include "Render.h"
#include "Config.h"

namespace Render
{
	void LyricRender::Init(HWND hwnd)
	{
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&m_pD2DFactory));
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pDWFactory);

		RecreateRT(hwnd);
	}

	void LyricRender::Draw()
	{
		if (!m_pRT)return;
		m_pRT->BeginDraw();
		m_pRT->Clear(D2D1::ColorF(0, 0, 0, 1.0));

		auto& cfg = Config::g_Config;
		auto size = m_pRT->GetSize();
		ComPtr<ID2D1SolidColorBrush> brush;

		auto alignment = cfg.CenterAlign ? DWRITE_TEXT_ALIGNMENT_CENTER : DWRITE_TEXT_ALIGNMENT_LEADING;

		//Draw Main Lyric
		ComPtr<IDWriteTextFormat> fmtOri;
		m_pDWFactory->CreateTextFormat(cfg.Font.c_str(), NULL, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, cfg.MainFontSize, L"zh-CN", &fmtOri);
		fmtOri->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		fmtOri->SetTextAlignment(alignment);
		m_pRT->CreateSolidColorBrush(D2D1::ColorF(cfg.MainFontColor, (cfg.MainFontColor >> 24) / 255.0f), &brush);

		if (cfg.Show_Trans.empty())
		{
			fmtOri->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			m_pRT->DrawTextW(cfg.Show_Ori.c_str(), cfg.Show_Ori.length(), fmtOri.Get(), D2D1::RectF(0, 0, size.width, size.height), brush.Get());
		}
		else
		{
			//Draw Main&Trans
			fmtOri->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			m_pRT->DrawTextW(cfg.Show_Ori.c_str(), cfg.Show_Ori.length(), fmtOri.Get(), D2D1::RectF(0, cfg.YPadding, size.width, size.height), brush.Get());

			ComPtr<IDWriteTextFormat> fmtTrans;
			m_pDWFactory->CreateTextFormat(cfg.Font.c_str(), NULL, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, cfg.TransFontSize, L"zh-CN", &fmtTrans);
			fmtTrans->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
			fmtTrans->SetTextAlignment(alignment);
			brush->SetColor(D2D1::ColorF(cfg.TransFontColor, (cfg.TransFontColor >> 24) / 255.0f));
			fmtTrans->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
			m_pRT->DrawTextW(cfg.Show_Trans.c_str(), cfg.Show_Trans.length(), fmtTrans.Get(), D2D1::RectF(0, 0, size.width, size.height - 2), brush.Get());
		}

		m_pRT->EndDraw();
	}
	void LyricRender::RecreateRT(HWND hwnd)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right, rc.bottom)),
			&m_pRT
		);

		m_pRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
	}
}
