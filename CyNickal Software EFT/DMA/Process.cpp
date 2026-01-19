#include "pch.h"

#include "DMA.h"

#include "Process.h"

bool Process::GetProcessInfo(DMA_Connection* Conn)
{
	std::println("Waiting for process {}..", ConstStrings::Game);

	m_PID = 0;

	while (true)
	{
		VMMDLL_PidGetFromName(Conn->GetHandle(), ConstStrings::Game.c_str(), &m_PID);

		if (m_PID)
		{
			std::println("Found process `{}` with PID {}", ConstStrings::Game, m_PID);
			PopulateModules(Conn);
			break;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return true;
}

const uintptr_t Process::GetBaseAddress() const
{
	using namespace ConstStrings;
	return m_Modules.at(Game).Base;
}

const uintptr_t Process::GetBaseSize() const
{
	using namespace ConstStrings;
	return m_Modules.at(Game).Size;
}

const uintptr_t Process::GetUnityAddress() const
{
	using namespace ConstStrings;
	return m_Modules.at(Unity).Base;
}

const uintptr_t Process::GetUnitySize() const
{
	using namespace ConstStrings;
	return m_Modules.at(Unity).Size;
}

const uintptr_t Process::GetAssemblyAddress() const
{
	using namespace ConstStrings;
	return m_Modules.at(GameAssembly).Base;
}

const uintptr_t Process::GetAssemblySize() const
{
	using namespace ConstStrings;
	return m_Modules.at(GameAssembly).Size;
}

const DWORD Process::GetPID() const
{
	return m_PID;
}

bool Process::ResolveModule(DMA_Connection* Conn, const std::string& moduleName)
{
	auto handle = Conn->GetHandle();

	PVMMDLL_MAP_MODULEENTRY entry{};
	std::wstring wName(moduleName.begin(), moduleName.end());

	if (!VMMDLL_Map_GetModuleFromNameW(handle, m_PID, const_cast<LPWSTR>(wName.c_str()), &entry, VMMDLL_MODULE_FLAG_NORMAL))
	{
		return false;
	}

	ModuleInfo info{};
	info.Base = entry->vaBase;
	info.Size = entry->cbImageSize;

	m_Modules[moduleName] = info;
	return true;
}

bool Process::PopulateModules(DMA_Connection* Conn)
{
	using namespace ConstStrings;

	auto Handle = Conn->GetHandle();

	const std::vector<std::string> modulesToLoad = { Game, Unity, GameAssembly };

	for (const auto& moduleName : modulesToLoad)
	{
		while (!m_Modules.contains(moduleName) || m_Modules[moduleName].Base == 0)
		{
			ResolveModule(Conn, moduleName);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	for (const auto& [Name, Info] : m_Modules)
	{
		std::println(
			"Module `{}` at address 0x{:X} (size: 0x{:X})",
			Name,
			Info.Base,
			Info.Size
		);
	}

	return true;
}
