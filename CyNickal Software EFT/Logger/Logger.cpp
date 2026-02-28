#include "pch.h"
#include "Logger.h"
#include <shlobj.h>
#include <io.h>
#include <fcntl.h>

static std::filesystem::path g_LogPath;

std::filesystem::path Logger::GetLogPath()
{
	return g_LogPath;
}

void Logger::Initialize()
{
	char docPath[MAX_PATH]{};
	std::filesystem::path logDir;
	if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, docPath)))
		logDir = std::filesystem::path(docPath) / "EFT-DMA";
	else
		logDir = std::filesystem::path(".");

	std::filesystem::create_directories(logDir);
	g_LogPath = logDir / "eft_dma.log";

	// Save the original stdout fd so we can keep writing to the console
	int origFd = _dup(_fileno(stdout));

	// Open the log file (overwrite each run)
	FILE* logFile = nullptr;
	if (fopen_s(&logFile, g_LogPath.string().c_str(), "w") != 0 || !logFile)
	{
		_close(origFd);
		return;
	}

	// Create an anonymous pipe — stdout writes go into the write end
	int pipeFds[2];
	if (_pipe(pipeFds, 65536, _O_TEXT) != 0)
	{
		fclose(logFile);
		_close(origFd);
		return;
	}

	// Redirect stdout → write end of pipe
	fflush(stdout);
	_dup2(pipeFds[1], _fileno(stdout));
	_close(pipeFds[1]);
	setvbuf(stdout, nullptr, _IONBF, 0); // unbuffered so writes reach the thread immediately

	// Background thread: drain the pipe and echo to both the console and the log file
	std::thread([pipeFds, origFd, logFile]()
	{
		char buf[4096];
		int n;
		while ((n = _read(pipeFds[0], buf, sizeof(buf))) > 0)
		{
			_write(origFd, buf, n);
			fwrite(buf, 1, n, logFile);
			fflush(logFile);
		}
		_close(pipeFds[0]);
		_close(origFd);
		fclose(logFile);
	}).detach();

	std::println("[Logger] Logging to: {}", g_LogPath.string());
}
