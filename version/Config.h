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

		bool AlwaysTop = false;
		bool CenterAlign = true;
		float XLeftMargin = 10.0f;
		float XRightMargin = 50.0f;
		float YPadding = 5.0f;

		std::wstring Show_Ori;
		std::wstring Show_Trans;
	};

	inline LyricConfig g_Config;
}