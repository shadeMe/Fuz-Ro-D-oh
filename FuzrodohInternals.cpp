#pragma warning(disable : 4005)
#include "FuzrodohInternals.h"

IDebugLog				gLog("Fuz Ro D'oh.log");
PluginHandle			g_pluginHandle = kPluginHandle_Invalid;
static const char*		kShimAssetFilePath = "Data\\Sound\\Voice\\Fuz Ro D'oh\\Stock.wav";

_DefineHookHdlr(TESTopicInfoGetAssetPath, 0x0066C708);

BSIStream* BSIStream::CreateInstance( const char* FilePath, BSResource::Location* ParentLocation /*= NULL*/ )
{
	void* Instance = cdeclCall<void*>(0x004017F0, 0x10);		// BSMemory::Allocate, standard bucket
	return thisCall<BSIStream*>(0x00AF09D0, Instance, FilePath, ParentLocation);
}

void BollocksBollocksBollocks()
{
	_MemHdlr(TESTopicInfoGetAssetPath).WriteJump();
}

void __stdcall SneakAtackVoicePath(char* VoicePathBuffer)
{
	if (strlen(VoicePathBuffer) < 17)
		return;

	std::string FUZPath(VoicePathBuffer), WAVPath(VoicePathBuffer);
	FUZPath.erase(0, 5);
	WAVPath.erase(0, 5);

	FUZPath.erase(FUZPath.length() - 3, 3);
	FUZPath.append("fuz");

	BSIStream* WAVStream = BSIStream::CreateInstance(WAVPath.c_str());
	BSIStream* FUZStream = BSIStream::CreateInstance(FUZPath.c_str());
	if (WAVStream->valid == 0 && FUZStream->valid == 0)
	{
		memcpy(VoicePathBuffer, kShimAssetFilePath, strlen(kShimAssetFilePath) + 1);
	}

	WAVStream->ReleaseInstance();
	FUZStream->ReleaseInstance();
}

#define _hhName	TESTopicInfoGetAssetPath
_hhBegin()
{
	_hhSetVar(Retn, 0x0066C70D);
	_hhSetVar(Call, 0x00A38CE0);	// StringCache::Ref::Set()
	__asm
	{
		mov		eax, [esp]
		pushad
		push	eax
		call	SneakAtackVoicePath
		popad

		call	[_hhGetVar(Call)]
		jmp		[_hhGetVar(Retn)]
	}
}
