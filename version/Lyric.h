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

	private:
		Lyric(const Lyric&) = delete;
		Lyric& operator=(const Lyric&) = delete;
		Lyric() = default;
		~Lyric() = default;

		std::string m_currentSongId;
		std::mutex m_mutex;
	};
}
