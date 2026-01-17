#include "Lyric.h"
#include "Utils.h"
#include "NetClient.h"

namespace LyricProc
{
	void Lyric::UpdateCurrentSong(const std::string& rawId)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		size_t pos = rawId.find('_');
		auto SongId = (pos != std::string::npos) ? rawId.substr(0, pos) : rawId;

		if (m_currentSongId == SongId) return;
		m_currentSongId = SongId;
		Utils::Logger::Log("Updated current songId to: {}", SongId);
		NetClient::Instance().RequestLyric(SongId);
	}

	std::string Lyric::GetCurrentSong()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_currentSongId;
	}

	void Lyric::UpdateCurrentTick(double tick)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_CurrentTick == tick) return;
		m_CurrentTick = tick;
		Utils::Logger::Log("Tick to: {}", m_CurrentTick);
	}


}