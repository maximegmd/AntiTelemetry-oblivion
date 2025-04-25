#include <PluginAPI.h>
#include <obse64_version.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <mem/mem.h>
#include <mem/module.h>
#include <mem/pattern.h>
#include <mem/protect.h>

#include <shlobj.h>
#include <windows.h>
#include <MinHook.h>

#include <filesystem>

#pragma comment(lib, "shell32.lib")


std::filesystem::path GetPath(const std::string_view aRelative)
{
	CHAR my_documents[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

	if (result != S_OK)
	{
		return ".\\OBSE\\Plugins\\";
	}
	
	auto path = std::filesystem::path(my_documents) / aRelative;

	create_directories(path);

	return path;
}

__int64 sendTelemetry_hook(__int64 a1, unsigned __int8 a2, __int64 a3, __int64 a4)
{
	spdlog::info("sendTelemetry_hook called");
	spdlog::default_logger()->flush();
	return 0;
}

extern "C" 
{
	__declspec(dllexport) OBSEPluginVersionData OBSEPlugin_Version =
	{
		OBSEPluginVersionData::kVersion,

		1,
		"Anti Telemetry",
		"yamashi",

		1,
		1 ,
		{ RUNTIME_VERSION_0_411_140, 0 },

		0,
		0, 0, 0
	};

	__declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface* obse)
	{
		spdlog::set_level(spdlog::level::info);
		spdlog::default_logger()->sinks().push_back(
			std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
				(GetPath("My Games\\Oblivion Remastered\\OBSE\\Logs") / "AntiTelemetry.log").string(), 1048576 * 5, 3
			)
		);

		//MH_Initialize();

		auto parent = mem::module::main();
		mem::region text;
		parent.enum_segments(
			[&](const mem::region& seg, const mem::prot_flags flags)
			{
				if (flags & mem::prot_flags::X)
				{
					text = seg;
				}
				return true;
			}
		);

		mem::pattern sendTelemetry("48 8B C4 55 41 56 48 8D 68 E8");
		auto sendTelemetryAddr = mem::scan(sendTelemetry, text);
		if (sendTelemetryAddr)
		{
			spdlog::info("Found sendTelemetry at {:#x}", sendTelemetryAddr.as<uintptr_t>());
			mem::region region(sendTelemetryAddr, 0x10);
			mem::protect _(region, mem::prot_flags::RWX);
			*sendTelemetryAddr.as<uint8_t*>() = 0xC3;

			/*auto result = MH_CreateHook(
				reinterpret_cast<LPVOID>(sendTelemetryAddr.as<uintptr_t>()),
				&sendTelemetry_hook,
				nullptr
			);

			spdlog::info("Hooked function {}", (int)result);*/
		}
		else
		{
			spdlog::info("Could not find sendTelemetry :'(");
		}

		//MH_EnableHook(MH_ALL_HOOKS);

		spdlog::default_logger()->flush();

		return true;
	}
};

