#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace LyricProc
{
	class NetClient
	{
	public:
		static	NetClient& Instance()
		{
			static  NetClient instance;
			return instance;
		}

		void RequestLyric(const std::string& SongId);

	private:
		NetClient(const NetClient&) = delete;
		NetClient& operator=(const NetClient&) = delete;
		NetClient()
		{
			m_worker = std::thread(&NetClient::WorkerLoop, this);
		}
		~NetClient()
		{
			m_running = false;
			m_cv.notify_all();
			if (m_worker.joinable()) m_worker.join();
		}

		std::mutex m_mtx;
		std::condition_variable m_cv;
		std::atomic<bool> m_running{ true };
		std::thread m_worker;
		std::string requestSongId;
		void WorkerLoop();
		void SendRequest(std::string SongId);
		void SaveJson(const std::string& songId, const std::string& content);
	};
}