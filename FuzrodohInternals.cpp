#pragma warning(disable : 4005)
#include "FuzrodohInternals.h"

IDebugLog				gLog("Fuz Ro D'oh.log");
PluginHandle			g_pluginHandle = kPluginHandle_Invalid;

FuzRoDohINIManager		FuzRoDohINIManager::Instance;
SubtitleHasher			SubtitleHasher::Instance;
const double			SubtitleHasher::kPurgeInterval = 1000.0 * 60.0f;

SME::INI::INISetting	kWordsPerSecondSilence("WordsPerSecondSilence",
											   "General",
											   "Number of words a second of silent voice can \"hold\"",
											   (SInt32)2);

SME::INI::INISetting	kSkipEmptyResponses("SkipEmptyResponses",
											"General",
											"Don't play back silent dialog for empty dialog responses",
											(SInt32)1);

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

	RegisterSetting(&kWordsPerSecondSilence);
	RegisterSetting(&kSkipEmptyResponses);

	if (CreateINI)
		Save();
}

SubtitleHasher::HashT SubtitleHasher::CalculateHash( const char* String )
{
	SME_ASSERT(String);

	// Uses the djb2 string hashing algorithm
	// http://www.cse.yorku.ca/~oz/hash.html

	HashT Hash = 0;
	int i;

	while (i = *String++)
		Hash = ((Hash << 5) + Hash) + i; // Hash * 33 + i

	return Hash;
}

void SubtitleHasher::Add( const char* Subtitle )
{
	if (Subtitle && strlen(Subtitle) > 1 && HasMatch(Subtitle) == false)
	{
		Store.push_back(CalculateHash(Subtitle));
	}
}

bool SubtitleHasher::HasMatch( const char* Subtitle )
{
	HashT Current = CalculateHash(Subtitle);

	return std::find(Store.begin(), Store.end(), Current) != Store.end();
}

void SubtitleHasher::Purge( void )
{
	Store.clear();
}

void SubtitleHasher::Tick( void )
{
	TickCounter.Update();
	TickReminder -= TickCounter.GetTimePassed();

	if (TickReminder <= 0.0f)
	{
		TickReminder = kPurgeInterval;

#ifndef NDEBUG
		_MESSAGE("SubtitleHasher::Tick - Tock!");
#endif
		// we need to periodically purge the hash store as we can't differentiate b'ween topic responses with the same dialog text but different voice assets
		// for instance, there may be two responses with the text "Hello there!" but only one with a valid voice file
		Purge();
	}
}

BSIStream* BSIStream::CreateInstance( const char* FilePath, BSResource::Location* ParentLocation /*= NULL*/ )
{
	void* Instance = FormHeap_Allocate(0x10);		// standard bucket
	return thisCall<BSIStream*>(0x00B08630, Instance, FilePath, ParentLocation);
}

MenuTopicManager* MenuTopicManager::GetSingleton( void )
{
	return *((MenuTopicManager**)0x013105D8);
}

bool MenuTopicManager::InitiateDialog( TESObjectREFR* Speaker, bool Arg2 /*= 0*/, TESTopicInfo* Topic, bool Arg4 /*= 0*/ )
{
	return thisCall<bool>(0x006758A0, Speaker, Arg2, Topic, Arg4);
}

void UIUtils::QueueMessage( const char* Message, UInt32 Arg2 /*= 0*/, bool Arg3 /*= true*/ )
{
	cdeclCall<void>(0x008997A0, Message, Arg2, Arg3);
}

_DefineHookHdlr(TESTopicInfoGetAssetPath, 0x00673268);
_DefineHookHdlr(ForceSubtitlesMark1, 0x008980F7);		// UIUtils::QueueDialogSubtitle
_DefineHookHdlr(ForceSubtitlesMark2, 0x00892C0B);		// ActorSoundCallbackManager::DisplayQueuedNPCChatterData (Dialog Subs)
_DefineHookHdlr(ForceSubtitlesMark3, 0x00892D21);		// ActorSoundCallbackManager::QueueNPCChatterData
_DefineHookHdlr(ForceSubtitlesMark4, 0x00892B1D);		// ActorSoundCallbackManager::DisplayQueuedNPCChatterData (General Subs)
_DefineHookHdlr(MainLoopTick, 0x0069CC0D);				// Main::GameLoop

void BollocksBollocksBollocks()
{
	_MemHdlr(TESTopicInfoGetAssetPath).WriteJump();
	_MemHdlr(ForceSubtitlesMark1).WriteJump();
	_MemHdlr(ForceSubtitlesMark2).WriteJump();
	_MemHdlr(ForceSubtitlesMark3).WriteJump();
	_MemHdlr(ForceSubtitlesMark4).WriteJump();
	_MemHdlr(MainLoopTick).WriteJump();
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
		static const int kWordsPerSecond = kWordsPerSecondSilence.GetData().i;
		static const int kMaxSeconds = 10;

		int SecondsOfSilence = 2;
		char ShimAssetFilePath[0x104] = {0};
		std::string ResponseText(Data->responseText.Get());

		if (ResponseText.length() > 4 && strncmp(ResponseText.c_str(), "<ID=", 4))
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

			// calculate the response text's hash and stash it for later lookups
			SubtitleHasher::Instance.Add(Data->responseText.Get());
		}

		if (ResponseText.length() > 1 || (ResponseText.length() == 1 && ResponseText[0] == ' ' && kSkipEmptyResponses.GetData().i == 0))
		{
			FORMAT_STR(ShimAssetFilePath, "Data\\Sound\\Voice\\Fuz Ro Doh\\Stock_%d.xwm", SecondsOfSilence);
			memcpy(VoicePathBuffer, ShimAssetFilePath, strlen(ShimAssetFilePath) + 1);
#ifndef NDEBUG
		_MESSAGE("Missing Asset - Switching to '%s'", ShimAssetFilePath);
#endif
		}
	}

	WAVStream->ReleaseInstance();
	FUZStream->ReleaseInstance();
	XWMStream->ReleaseInstance();
}

#define _hhName	TESTopicInfoGetAssetPath
_hhBegin()
{
	_hhSetVar(Retn, 0x0067326D);
	_hhSetVar(Call, 0x00A51210);	// StringCache::Ref::Set()
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

bool __stdcall GetShouldForceSubs(NPCChatterData* ChatterData, UInt32 ForceRegardless, const char* Subtitle)
{
	bool Result = false;

	if (Subtitle && SubtitleHasher::Instance.HasMatch(Subtitle))		// force if the subtitle is for a voiceless response
	{
#ifndef NDEBUG
		_MESSAGE("Found a match for %s - Forcing subs", Subtitle);
#endif

		Result = true;
	}
	else if (ForceRegardless || (ChatterData && ChatterData->forceSubtitles))
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
	_hhSetVar(Retn, 0x00898100);
	_hhSetVar(Jump, 0x0089817F);
	_hhSetVar(Call, 0x00892710);	// INI::bDialogueSubtitles_Interface::Get()
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
		popad
		mov		eax, [esp + 0x0C]
		pushad
		push	eax
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
	_hhSetVar(Retn, 0x00892C12);
	_hhSetVar(Jump, 0x00892C3C);
	_hhSetVar(INISetting, 0x012B44C4);
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
		push	[ebp + 0x4]
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
	_hhSetVar(Retn, 0x00892D2A);
	_hhSetVar(Jump, 0x00892D85);
	_hhSetVar(INISetting, 0x012B44C4);
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
		popad
		mov		eax, [esp + 0x2C]
		mov		edx, [esp + 0x30]
		pushad
		push	eax
		push	edx
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
	_hhSetVar(Retn, 0x00892B2A);
	_hhSetVar(Jump, 0x00892C0B);
	_hhSetVar(INISetting, 0x012B44B8);
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
		push	[ebp + 0x4]
		push	0
		push	ebp
		call	GetShouldForceSubs
		test	al, al
		jnz		FORCESUBS
		popad
		jmp		[_hhGetVar(Jump)]
	}
}

void __stdcall PerformHouseKeeping(void)
{
	SubtitleHasher::Instance.Tick();
}

#define _hhName	MainLoopTick
_hhBegin()
{
	_hhSetVar(Retn, 0x0069CC12);
	_hhSetVar(Call, 0x00746DD0);
	__asm
	{
		call	[_hhGetVar(Call)]

		pushad
		call	PerformHouseKeeping
		popad

		jmp		[_hhGetVar(Retn)]
	}
}
