#pragma once

#include <fstream>
#include <mutex>
#include <string>

namespace SvgLib::Core {

	enum class LogLevel { L_DEBUG, L_INFO, L_WARNING, L_ERROR };

	class DebugLogger {
	public:
		// Singleton
		static DebugLogger& Instance();
	
		void Log(LogLevel level, const std::string& message);
		void Log(LogLevel level, const std::wstring& message);

		std::string WStringToUTF8(const std::wstring& wstr);
		
		static void Debug(const std::string& message) { Instance().Log(LogLevel::L_DEBUG, message); }
		static void Debug(const std::wstring& message) { Instance().Log(LogLevel::L_DEBUG, message); }
		
		static void Info(const std::string& message) { Instance().Log(LogLevel::L_INFO, message); }
		static void Info(const std::wstring& message) { Instance().Log(LogLevel::L_INFO, message); }
		
		static void Warning(const std::string& message) { Instance().Log(LogLevel::L_WARNING, message); }
		static void Warning(const std::wstring& message) { Instance().Log(LogLevel::L_WARNING, message); }
		
		static void Error(const std::string& message) { Instance().Log(LogLevel::L_ERROR, message); }
		static void Error(const std::wstring& message) { Instance().Log(LogLevel::L_ERROR, message); }

		// Prevent copies of DebugLogger
		DebugLogger(const DebugLogger&) = delete;
		DebugLogger& operator=(const DebugLogger&) = delete;

	private:
		DebugLogger();
		~DebugLogger();

		std::ofstream logFile_;
		std::mutex mutex_;

		std::string CurrentTime();
	};

}