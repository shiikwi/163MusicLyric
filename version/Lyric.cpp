#define _CRT_SECURE_NO_WARNINGS
#include "Lyric.h"
#include "Utils.h"
#include "NetClient.h"
#include "Config.h"
#include "3rd/json.hpp"
#include <sstream>
#include <regex>
#include <algorithm>
using json = nlohmann::json;

namespace LyricProc
{
	void Lyric::UpdateCurrentSong(const std::string& rawId)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		size_t pos = rawId.find('_');
		auto SongId = (pos != std::string::npos) ? rawId.substr(0, pos) : rawId;

		if (m_currentSongId == SongId) return;
		m_currentSongId = SongId;
		Utils::Logger::Log("Updated current songId to: {}", SongId);
		NetClient::Instance().RequestLyric(SongId);
	}

	std::string Lyric::GetCurrentSong()
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		return m_currentSongId;
	}

	void Lyric::UpdateCurrentTick(double tick)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		if (m_CurrentTick == tick) return;
		m_CurrentTick = tick;
		//Utils::Logger::Log("Tick to: {}", m_CurrentTick);
		if (m_lyricMap.empty())return;
		auto lyr = Lyric::GetCurrentLyricLine(tick);
		static std::string lastOri;
		if (lyr.ori != lastOri)
		{
			Config::g_Config.Show_Ori = Utils::UTF82WString(lyr.ori);
			Config::g_Config.Show_Trans = lyr.trans.empty() ? L"" : Utils::UTF82WString(lyr.trans);
			lastOri = lyr.ori;
		}
	}

	void Lyric::ParseLyricJson(const std::string& jsonStr)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		m_lyricMap.clear();
		m_LyricsJson = jsonStr;
		if (m_LyricsJson.empty())
		{
			m_lyricMap[0.0] = { (const char*)u8"等待获取歌词中...", "" };
			return;
		}
		try
		{
			auto j = json::parse(m_LyricsJson);
			if (j.value("pureMusic", false))
			{
				m_lyricMap[0.0] = { (const char*)u8"纯音乐，请欣赏", "" };
				return;
			}

			if (j.contains("lrc") && j["lrc"].contains("lyric"))
			{
				ParseLyricLine(j["lrc"]["lyric"].get<std::string>(), false);
			}

			if (j.contains("tlyric") && j["tlyric"].contains("lyric"))
			{
				ParseLyricLine(j["tlyric"]["lyric"].get<std::string>(), true);
			}
		}
		catch (...) {}
	}

	void Lyric::ParseLyricLine(const std::string& lrcStr, bool isTrans)
	{
		std::stringstream ss(lrcStr);
		std::string line;
		std::regex timeRegex(R"(\[(\d+:\d+\.\d+)\](.*))");
		std::smatch match;

		while (std::getline(ss, line))
		{
			if (line.empty()) continue;

			if (line[0] == '{')
			{
				try
				{
					auto jline = json::parse(line);
					double time = jline.value("t", 0) / 1000.0;
					std::string content = "";
					for (auto& item : jline["c"])
					{
						if (item.contains("tx")) content += item["tx"].get<std::string>();
					}

					if (!content.empty())
					{
						if (isTrans) m_lyricMap[time].trans = content;
						else m_lyricMap[time].ori = content;
					}
				}
				catch (json::parse_error& e) { Utils::Logger::Log("Json parse error"); }
			}
			else if (std::regex_search(line, match, timeRegex))
			{
				double time = ParseTimeStamp(match[1]);
				std::string content = match[2];

				if (content.find("[by:") != std::string::npos) continue;

				if (isTrans) m_lyricMap[time].trans = content;
				else m_lyricMap[time].ori = content;
			}
		}
	}

	double Lyric::ParseTimeStamp(const std::string& timeStr)
	{
		float min = 0, sec = 0;
		if (sscanf(timeStr.c_str(), "%f:%f", &min, &sec) == 2)
		{
			return min * 60.0 + sec;
		}
		return 0.0;
	}

	LyricLine Lyric::GetCurrentLyricLine(double currtime)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		auto it = m_lyricMap.upper_bound(currtime);
		if (it == m_lyricMap.begin())
			return it->second;
		return (--it)->second;
	}

}