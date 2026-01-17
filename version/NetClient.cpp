#include "NetClient.h"
#include "Utils.h"
#include "cpr/cpr.h"
#include <fstream>

namespace LyricProc
{
	void NetClient::RequestLyric(const std::string& SongId)
	{
		{
			std::lock_guard<std::mutex> lock(m_mtx);
			requestSongId = SongId;
		}
		m_cv.notify_one();
	}

	void NetClient::WorkerLoop()
	{
		while (m_running)
		{
			std::string NowTask;
			{
				std::unique_lock<std::mutex> lock(m_mtx);
				m_cv.wait(lock, [this] {return !requestSongId.empty() || !m_running;});
				if (!m_running)break;
				NowTask = std::move(requestSongId);
				requestSongId.clear();
			}
			if (!NowTask.empty())
				SendRequest(NowTask);
		}
	}

	void NetClient::SendRequest(std::string SongId)
	{
		Utils::Logger::Log("Send HTTPS request for {}", SongId);

		cpr::Parameters params
		{
			{"os", "pc"},
			{"id", SongId},
			{"lv", "-1"},
			{"kv", "-1"},
			{"tv", "-1"}
		};

		auto res = cpr::Get(
			cpr::Url{ "https://music.163.com/api/song/lyric/v1" },
			params,
			cpr::Header(),
			cpr::Timeout{ 5000 }
		);
		if (res.status_code == 200)
		{
			Utils::Logger::Log("HTTPS Request Success");

			SaveJson(SongId, res.text);
		}
		else
		{
			Utils::Logger::Error("HTTPS Request Failed: {}, {}", res.status_code, res.error.message);
		}
	}

	void NetClient::SaveJson(const std::string& songId, const std::string& content)
	{
		auto fileName = "lyric_" + songId + ".json";
		std::ofstream out(fileName);
		if (out.is_open())
		{
			out << content;
			out.close();
			Utils::Logger::Log("{} Json Save Success", fileName);
		}
	}

}

