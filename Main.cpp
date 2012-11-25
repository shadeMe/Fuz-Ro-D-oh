#include "FuzrodohInternals.h"
#include "VersionInfo.h"

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		_MESSAGE("Fuz Ro D'oh Initializing...");

		info->infoVersion =	PluginInfo::kInfoVersion;
		info->name =		"Fuz Ro D'oh";
		info->version =		PACKED_SME_VERSION;

		g_pluginHandle = skse->GetPluginHandle();

		if(skse->isEditor)
			return false;
		else if(skse->runtimeVersion != RUNTIME_VERSION_1_8_151_0)
		{
			_MESSAGE("Unsupported runtime version %08X", skse->runtimeVersion);
			return false;
		}

		// supported runtime version
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		_MESSAGE("Initializing INI Manager");
		g_INIManager->Initialize("Data\\SKSE\\Plugins\\Fuz Ro D'oh.ini", NULL);

		BollocksBollocksBollocks();

		_MESSAGE("Fuz Ro D'oh Initialized!");
		return true;
	}
};