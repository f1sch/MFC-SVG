#include "pch.h"
#include "DebugLogger.h"

#include <corecrt.h>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <string>

using namespace SvgLib::Core;

static std::filesystem::path DefaultLogPath()
{
	wchar_t exePath[MAX_PATH]{};
	DWORD len = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	std::filesystem::path exe = (len > 0) ? std::filesystem::path(exePath) : std::filesystem::current_path();
	std::filesystem::path dir = exe.has_parent_path() ? exe.parent_path() : std::filesystem::current_path();
	return dir / L"Logs" / L"debugLogger.log";
}

DebugLogger& DebugLogger::Instance()
{
	static DebugLogger instance;
	return instance;
}

DebugLogger::DebugLogger()
{
	try
	{
		const auto logPath = DefaultLogPath();
		std::error_code ec;
		std::filesystem::create_directories(logPath.parent_path(), ec);
		logFile_.open(logPath, std::ios::app);
	}
	catch (...)
	{
		// Keep logging disabled.
	}
}

DebugLogger::~DebugLogger()
{
	if (logFile_.is_open())
		logFile_.close();
}

std::string DebugLogger::CurrentTime()
{
	std::time_t now = std::time(nullptr);

	std::tm tmRes;
	if (localtime_s(&tmRes, &now) != 0)
	{
		return "";
	}

	char buf[20];
	if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmRes))
	{
		return buf;
	}
	return "";
}

void DebugLogger::Log(LogLevel level, const std::string& message)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (!logFile_.is_open())
		return;

	static const char* levelNames[] = {
		"DEBUG", "INFO", "WARNING", "ERROR"
	};
	const char* lvl = levelNames[static_cast<int>(level)];

	logFile_ << "[" << CurrentTime() << "] " << "[" << lvl << "] "
		<< message << "\n";

	logFile_.flush();
}

void SvgLib::Core::DebugLogger::Log(LogLevel level, const std::wstring& message)
{
	Log(level, WStringToUTF8(message));
}

std::string SvgLib::Core::DebugLogger::WStringToUTF8(const std::wstring& wstr)
{
	if (wstr.empty()) return {};

	int size_needed = WideCharToMultiByte(
		CP_UTF8, 0,
		wstr.data(), (int)wstr.size(),
		nullptr, 0,
		nullptr, nullptr
	);

	std::string str(size_needed, 0);

	WideCharToMultiByte(
		CP_UTF8, 0,
		wstr.data(), (int)wstr.size(),
		str.data(), size_needed,
		nullptr, nullptr
	);
	return str;
}
