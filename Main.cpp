#include "FuzrodohInternals.h"

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		_MESSAGE("Fuz Ro D'oh Initializing...");

		info->infoVersion =	PluginInfo::kInfoVersion;
		info->name =		"Fuz Ro D'oh";
		info->version =		1;

		g_pluginHandle = skse->GetPluginHandle();

		if(skse->isEditor)
			return false;
		else if(skse->runtimeVersion != RUNTIME_VERSION_1_4_21_0)
		{
			_MESSAGE("Unsupported runtime version %08X", skse->runtimeVersion);
			return false;
		}

		// supported runtime version
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		BollocksBollocksBollocks();
		_MESSAGE("Fuz Ro D'oh Initialized!");
		return true;
	}
};