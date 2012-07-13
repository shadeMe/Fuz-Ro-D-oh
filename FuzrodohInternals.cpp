#pragma warning(disable : 4005)
#include "FuzrodohInternals.h"

IDebugLog				gLog("Fuz Ro D'oh.log");
PluginHandle			g_pluginHandle = kPluginHandle_Invalid;
INI::INIManager*		g_INIManager = new FuzRoDohINIManager();

void FuzRoDohINIManager::Initialize( const char* INIPath, void* Paramenter )
{
	this->INIFilePath = INIPath;
	_MESSAGE("INI Path: %s", INIPath);

	std::fstream INIStream(INIPath, std::fstream::in);
	bool CreateINI = false;

	if (INIStream.fail())
	{
		_MESSAGE("INI File not found; Creating one...");
		CreateINI = true;
	}

	INIStream.close();
	INIStream.clear();

	RegisterSetting("WordsPerSecondSilence", "General", "2", "Number of words a second of silent voice can \"hold\"");

	if (CreateINI)
		Save();
	else
		Load();
}

BSIStream* BSIStream::CreateInstance( const char* FilePath, BSResource::Location* ParentLocation /*= NULL*/ )
{
	void* Instance = FormHeap_Allocate(0x10);		// standard bucket
	return thisCall<BSIStream*>(0x00B01140, Instance, FilePath, ParentLocation);
}

MenuTopicManager* MenuTopicManager::GetSingleton( void )
{
	return *((MenuTopicManager**)0x01305E68);
}

bool MenuTopicManager::InitiateDialog( TESObjectREFR* Speaker, bool Arg2 /*= 0*/, TESTopicInfo* Topic, bool Arg4 /*= 0*/ )
{
	return thisCall<bool>(0x006744B0, Speaker, Arg2, Topic, Arg4);
}

void UIUtils::QueueMessage( const char* Message, UInt32 Arg2 /*= 0*/, bool Arg3 /*= true*/ )
{
	cdeclCall<void>(0x00892C10, Message, Arg2, Arg3);
}

_DefineHookHdlr(TESTopicInfoGetAssetPath, 0x00671E78);
_DefineHookHdlr(ForceSubtitlesMark1, 0x00891567);		// UIUtils::QueueDialogSubtitle
_DefineHookHdlr(ForceSubtitlesMark2, 0x0088C16B);		// ActorSoundCallbackManager::DisplayQueuedNPCChatterData (Dialog Subs)
_DefineHookHdlr(ForceSubtitlesMark3, 0x0088C281);		// ActorSoundCallbackManager::QueueNPCChatterData
_DefineHookHdlr(ForceSubtitlesMark4, 0x0088C07D);		// ActorSoundCallbackManager::DisplayQueuedNPCChatterData (General Subs)

void BollocksBollocksBollocks()
{
	_MemHdlr(TESTopicInfoGetAssetPath).WriteJump();
	_MemHdlr(ForceSubtitlesMark1).WriteJump();
	_MemHdlr(ForceSubtitlesMark2).WriteJump();
	_MemHdlr(ForceSubtitlesMark3).WriteJump();
	_MemHdlr(ForceSubtitlesMark4).WriteJump();
}

void __stdcall SneakAtackVoicePath(char* VoicePathBuffer, CachedResponseData* Data, TESTopicInfo* TopicInfo)
{
	if (strlen(VoicePathBuffer) < 17)
		return;

	std::string FUZPath(VoicePathBuffer), WAVPath(VoicePathBuffer), XWMPath(VoicePathBuffer);
	WAVPath.erase(0, 5);

	FUZPath.erase(0, 5);
	FUZPath.erase(FUZPath.length() - 3, 3);
	FUZPath.append("fuz");

	XWMPath.erase(0, 5);
	XWMPath.erase(XWMPath.length() - 3, 3);
	XWMPath.append("xwm");

	BSIStream* WAVStream = BSIStream::CreateInstance(WAVPath.c_str());
	BSIStream* FUZStream = BSIStream::CreateInstance(FUZPath.c_str());
	BSIStream* XWMStream = BSIStream::CreateInstance(XWMPath.c_str());

#if 0
	_MESSAGE("Expected: %s", VoicePathBuffer);
	gLog.Indent();
	_MESSAGE("WAV Stream [%s] Validity = %d", WAVPath.c_str(), WAVStream->valid);
	_MESSAGE("FUZ Stream [%s] Validity = %d", FUZPath.c_str(), FUZStream->valid);
	_MESSAGE("XWM Stream [%s] Validity = %d", XWMPath.c_str(), XWMStream->valid);
	gLog.Outdent();
#endif

	if (WAVStream->valid == 0 && FUZStream->valid == 0 && XWMStream->valid == 0)
	{
		static const int kWordsPerSecond = g_INIManager->GetINIInt("WordsPerSecondSilence", "General");
		static const int kMaxSeconds = 10;

		int SecondsOfSilence = 1;
		char ShimAssetFilePath[0x104] = {0};
		std::string ResponseText(Data->responseText.Get());

		if (ResponseText.length() > 4)
		{
			SME::StringHelpers::Tokenizer TextParser(ResponseText.c_str(), " ");
			int WordCount = 0;

			while (TextParser.NextToken(ResponseText) != -1)
				WordCount++;

			SecondsOfSilence = WordCount / ((kWordsPerSecond > 0) ? kWordsPerSecond : 2) + 1;

			if (SecondsOfSilence <= 0)
				SecondsOfSilence = 2;
			else if (SecondsOfSilence > kMaxSeconds)
				SecondsOfSilence = kMaxSeconds;
		}

		FORMAT_STR(ShimAssetFilePath, "Data\\Sound\\Voice\\Fuz Ro Doh\\Stock_%d.xwm", SecondsOfSilence);
		memcpy(VoicePathBuffer, ShimAssetFilePath, strlen(ShimAssetFilePath) + 1);

#if 0
		_MESSAGE("Missing Asset - Switching to '%s'", ShimAssetFilePath);
#endif
	}

	WAVStream->ReleaseInstance();
	FUZStream->ReleaseInstance();
	XWMStream->ReleaseInstance();
}

#define _hhName	TESTopicInfoGetAssetPath
_hhBegin()
{
	_hhSetVar(Retn, 0x00671E7D);
	_hhSetVar(Call, 0x00A48D50);	// StringCache::Ref::Set()
	__asm
	{
		mov		eax, [esp]
		pushad
		push	ebp
		push	esi
		push	eax
		call	SneakAtackVoicePath
		popad

		call	[_hhGetVar(Call)]
		jmp		[_hhGetVar(Retn)]
	}
}

bool __stdcall GetShouldForceSubs(NPCChatterData* ChatterData, UInt32 ForceRegardless)
{
	bool Result = false;

	if (ForceRegardless || (ChatterData && ChatterData->forceSubtitles))
		Result = true;
	else
	{
		TESTopicInfo* CurrentTopicInfo = NULL;
		PlayerDialogData* Selection = NULL;

		if (MenuTopicManager::GetSingleton()->selectedResponseNode)
			Selection = MenuTopicManager::GetSingleton()->selectedResponseNode->Head.Data;
		else
			Selection = MenuTopicManager::GetSingleton()->lastSelectedResponse;

		if (Selection)
			CurrentTopicInfo = Selection->parentTopicInfo;
		else if (MenuTopicManager::GetSingleton()->rootTopicInfo)
			CurrentTopicInfo = MenuTopicManager::GetSingleton()->rootTopicInfo;
		else
			CurrentTopicInfo = MenuTopicManager::GetSingleton()->unk14;

		if (CurrentTopicInfo)
		{
			UInt16& Flags = *((UInt16*)((UInt32)CurrentTopicInfo + 0x24));

			if ((Flags >> 9) & 1)		// force subs flag's set
				Result = true;
		}
	}

	return Result;
}

#define _hhName	ForceSubtitlesMark1
_hhBegin()
{
	_hhSetVar(Retn, 0x00891570);
	_hhSetVar(Jump, 0x008915EF);
	_hhSetVar(Call, 0x0088BC70);	// INI::bDialogueSubtitles_Interface::Get()
	__asm
	{
		pushad
		call	[_hhGetVar(Call)]
		test	al, al
		jz		SUBSOFF
	FORCESUBS:
		popad
		jmp		[_hhGetVar(Retn)]
	SUBSOFF:
		push	0
		push	0
		call	GetShouldForceSubs
		test	al, al
		jnz		FORCESUBS
		popad
		jmp		[_hhGetVar(Jump)]
	}
}

#define _hhName	ForceSubtitlesMark2
_hhBegin()
{
	_hhSetVar(Retn, 0x0088C174);
	_hhSetVar(Jump, 0x0088C19C);
	_hhSetVar(INISetting, 0x012A9B7C);
	__asm
	{
		pushad
		mov		eax, [_hhGetVar(INISetting)]
		cmp		byte ptr [eax + 0x4], 0
		jz		DIALOGSUBSOFF
	FORCESUBS:
		popad
		jmp		[_hhGetVar(Retn)]
	DIALOGSUBSOFF:
		push	0
		push	ebp
		call	GetShouldForceSubs
		test	al, al
		jnz		FORCESUBS
		popad
		jmp		[_hhGetVar(Jump)]
	}
}

#define _hhName	ForceSubtitlesMark3
_hhBegin()
{
	_hhSetVar(Retn, 0x0088C28A);
	_hhSetVar(Jump, 0x0088C2E5);
	_hhSetVar(INISetting, 0x012A9B7C);
	__asm
	{
		pushad
		mov		eax, [_hhGetVar(INISetting)]
		cmp		byte ptr [eax + 0x4], 0
		jz		SUBSOFF
	FORCESUBS:
		popad
		jmp		[_hhGetVar(Retn)]
	SUBSOFF:
		push	[esp + 0x30]
		push	0
		call	GetShouldForceSubs
		test	al, al
		jnz		FORCESUBS
		popad
		jmp		[_hhGetVar(Jump)]
	}
}

#define _hhName	ForceSubtitlesMark4
_hhBegin()
{
	_hhSetVar(Retn, 0x0088C08A);
	_hhSetVar(Jump, 0x0088C16B);
	_hhSetVar(INISetting, 0x012A9B70);
	__asm
	{
		pushad
		mov		eax, [_hhGetVar(INISetting)]
		cmp		byte ptr [eax + 0x4], 0
		jz		GENERALSUBSOFF
	FORCESUBS:
		popad
		jmp		[_hhGetVar(Retn)]
	GENERALSUBSOFF:
		push	0
		push	ebp
		call	GetShouldForceSubs
		test	al, al
		jnz		FORCESUBS
		popad
		jmp		[_hhGetVar(Jump)]
	}
}