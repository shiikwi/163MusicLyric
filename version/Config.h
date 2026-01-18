#pragma once
#include <string>

namespace Config
{
	struct LyricConfig
	{
		std::wstring Font = L"Microsoft YaHei";

		float MainFontSize = 16.0f;

		float TransFontSize = 14.0f;

		unsigned int MainFontColor = 0xFFFFFFFF;

		unsigned int TransFontColor = 0xB0FFFFFF;

		int XLeftMargin = 5.0f;
		int XRightMargin = 5.0f;
		float YPadding = 0.5f;

		std::wstring Show_Ori;
		std::wstring Show_Trans;
	};

	inline LyricConfig g_Config;
}