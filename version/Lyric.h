#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <map>

namespace LyricProc
{
	struct LyricLine
	{
		std::string ori;
		std::string trans;
	};

	class Lyric
	{
	public:
		static Lyric& Instance()
		{
			static Lyric instance;
			return instance;
		}

		void UpdateCurrentSong(const std::string& songId);
		std::string GetCurrentSong();
		void UpdateCurrentTick(double tick);
		void ParseLyricJson(const std::string& jsonStr);
		LyricLine GetCurrentLyricLine(double currtime);

	private:
		Lyric(const Lyric&) = delete;
		Lyric& operator=(const Lyric&) = delete;
		Lyric() = default;
		~Lyric() = default;

		double m_CurrentTick;
		std::string m_currentSongId;
		std::recursive_mutex m_mutex;
		std::string m_LyricsJson;
		std::map<double, LyricLine>m_lyricMap;
		void ParseLyricLine(const std::string& lrcStr, bool isTrans);
		double ParseTimeStamp(const std::string& timeStr);
	};
}
