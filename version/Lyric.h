#pragma once
#include "Utils.h"
#include <mutex>

namespace LyricProc
{
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

	private:
		Lyric(const Lyric&) = delete;
		Lyric& operator=(const Lyric&) = delete;
		Lyric() = default;
		~Lyric() = default;

		double m_CurrentTick;
		std::string m_currentSongId;
		std::mutex m_mutex;
	};
}
