#include "Lyric.h"

namespace LyricProc
{
	void Lyric::UpdateCurrentSong(const std::string& songId)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_currentSongId == songId) return;

		m_currentSongId = songId;
		Utils::Logger::Log("Updated current songId to: {}", songId);
	}

	std::string Lyric::GetCurrentSong()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_currentSongId;
	}


}