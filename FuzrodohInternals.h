#pragma once

#include "skse/PluginAPI.h"
#include "skse/skse_version.h"
#include "skse/GameAPI.h"
#include "skse/GameTypes.h"
#include "skse/GameForms.h"
#include "skse/GameObjects.h"
#include "skse/GameReferences.h"
#include "skse/GameEvents.h"

#include "[Libraries]\SME Sundries\SME_Prefix.h"
#include "[Libraries]\SME Sundries\MemoryHandler.h"
#include "[Libraries]\SME Sundries\INIManager.h"
#include "[Libraries]\SME Sundries\StringHelpers.h"
#include "[Libraries]\SME Sundries\MiscGunk.h"

using namespace SME;
using namespace SME::MemoryHandler;

extern IDebugLog						gLog;
extern PluginHandle						g_pluginHandle;

extern SME::INI::INISetting				kWordsPerSecondSilence;

class FuzRoDohINIManager : public INI::INIManager
{
public:
	void								Initialize(const char* INIPath, void* Paramenter);

	static FuzRoDohINIManager			Instance;
};

class SubtitleHasher
{
	static const double					kPurgeInterval;		// in ms

	typedef unsigned long				HashT;
	typedef std::list<HashT>			HashListT;

	HashListT							Store;
	SME::MiscGunk::ElapsedTimeCounter	TickCounter;
	double								TickReminder;

	static HashT						CalculateHash(const char* String);

	void								Purge(void);
public:
	SubtitleHasher() : Store(), TickCounter(), TickReminder(kPurgeInterval) {}

	void								Add(const char* Subtitle);
	bool								HasMatch(const char* Subtitle);
	void								Tick(void);

	static SubtitleHasher				Instance;
};

#pragma region Deprecated
// deprecated in Skyrim, it seems - Beth has overhauled their resource managing codebase
// but used in the editor?
// 0C
class FileFinder
{
public:
	enum
	{
		kFileStatus_NotFound = 0,
		kFileStatus_Unpacked,
		kFileStatus_Packed
	};

	// members
	/*00*/ tArray<char*>	searchPathArray;

	// methods
	void					AddSearchPath(const char* SearchPath);
	UInt32					FindFile(const char* FilePath, char* OutAbsolutePath = NULL, UInt32 Arg3 = 0, int Arg4 = -1);	// absolute path buffer size >= MAX_PATH
};
STATIC_ASSERT(sizeof(FileFinder) == 0xC);
#pragma endregion

// 04+
class BSResource
{
public:
	// 04
	class Location
	{
		// members
		///*00*/ void**				vtbl;

		// methods
		virtual void*				Dtor(bool FreeMemory = true) = 0;
	};

	// 14
	class LooseFileLocation : public Location
	{
	public:
		// members
		//     /*00*/ Location
		/*04*/ UInt8				unk04;
		/*05*/ UInt8				pad04[3];
		/*08*/ StringCache::Ref		locationPath;
		/*0C*/ UInt32				unk0C;		// probably flags, init to 0x10000 or 0x200
		/*10*/ UInt8				unk10;
		/*11*/ UInt8				pad11[3];
	};

	// members
	///*00*/ void**					vtbl;

	// methods
	virtual void*					Dtor(bool FreeMemory = true) = 0;
};
STATIC_ASSERT(sizeof(BSResource::Location) == 0x4);
STATIC_ASSERT(sizeof(BSResource::LooseFileLocation) == 0x14);

// actually derives from BSTIOStream::TIStream<struct BSIOStreamTraits>
// 10
class BSIStream
{
public:
	// members
	///*00*/ void**					vtbl;
	/*04*/ BSTSmartPointer<void>	unk04;		// actual file stream
	/*08*/ UInt8					valid;		// set to 1 if the stream's valid
	/*09*/ UInt8					pad09[3];
	/*0C*/ StringCache::Ref			filePath;	// relative to the Data directory when no BSResource::Location's passed to the ctor (the game uses a static instance)
												// otherwise, use its location
	// methods
	virtual void*					ReleaseInstance(bool FreeMemory = true) = 0;		// dtor actually

	static BSIStream*				CreateInstance(const char* FilePath, BSResource::Location* ParentLocation = NULL);
};
STATIC_ASSERT(sizeof(BSIStream) == 0x10);

// 08
template <typename NodeT>
class BSSimpleList
{
public:
	template <typename NodeT>
	struct ListNode
	{
		// members
		/*00*/ NodeT*					Data;
		/*04*/ ListNode<NodeT>*			Next;
	};

	// members
	/*00*/ ListNode<NodeT>				Head;
};

// final cache that gets passed to the dialog playback subsystem
// 24
class CachedResponseData
{
public:
	// members
	/*00*/ BSString					responseText;
	/*08*/ UInt32					emotionType;
	/*0C*/ UInt32					emotionLevel;
	/*10*/ StringCache::Ref			voiceFilePath;		// relative path to the voice file
	/*14*/ UInt32					unk14;				// speaker idle anim?
	/*18*/ UInt32					unk18;				// listener idle anim?
	/*20*/ UInt8					useEmotionAnim;
	/*21*/ UInt8					hasLipFile;
	/*22*/ UInt8					pad22[2];
};

// arbitrary name, used to queue subtitles for gamemode conversations (outside the standard dialog menu, NPC-NPC or NPC-PC)
// 14
class NPCChatterData
{
public:
	// members
	/*00*/ UInt32					speaker;				// the BSHandleRefObject handle to the speaker
	/*04*/ BSString					title;
	/*0C*/ float					subtitleDistance;		// init to float::MAX
	/*10*/ UInt8					forceSubtitles;
	/*11*/ UInt8					pad11[3];
};

class PlayerDialogData;

typedef BSSimpleList<CachedResponseData>	CachedResponseListT;
typedef BSSimpleList<PlayerDialogData>		PlayerTopicListT;
typedef tArray<BGSDialogueBranch>			DialogBranchArrayT;
typedef tArray<TESTopic>					TopicArrayT;

// arbitrary name, the actual class is probably a member of MenuTopicManager and not limited to the player
// 2C
class PlayerDialogData
{
public:
	// members
	/*00*/ BSString					title;
	/*08*/ UInt8					unk08;
	/*09*/ UInt8					unk09;
	/*0A*/ UInt8					unk0A;
	/*0B*/ UInt8					pad0B;
	/*0C*/ CachedResponseListT		responses;
	/*14*/ TESQuest*				parentQuest;
	/*18*/ TESTopicInfo*			parentTopicInfo;
	/*1C*/ TESTopic*				parentTopic;
	/*20*/ CachedResponseListT*		unk20;				// seen pointing to this::unk0C
	/*24*/ UInt8					unk24;
	/*25*/ UInt8					pad25;
	/*26*/ UInt8					unk26;
	/*27*/ UInt8					pad27;
	/*28*/ TESTopic*				unk28;				// seen caching parentTopic
};
STATIC_ASSERT(sizeof(PlayerDialogData) == 0x2C);

// 78
class MenuTopicManager
{
public:
	// members
	///*00*/ void**					vtbl;
	/*04*/ BSTEventSink<void*>		unk04;					// BSTEventSink<PositionPlayerEvent>
	/*08*/ UInt32					unk08;
	/*0C*/ PlayerTopicListT*		selectedResponseNode;	// points to the ListNode that refers to the PlayerDialogData instance of the selected topicinfo
	/*10*/ PlayerTopicListT*		availableResponses;
	/*14*/ TESTopicInfo*			unk14;
	/*18*/ TESTopicInfo*			rootTopicInfo;
	/*1C*/ PlayerDialogData*		lastSelectedResponse;
	/*20*/ CRITICAL_SECTION			topicManagerCS;
	/*38*/ UInt32					speaker;				// a BSHandleRefObject handle to the speaker
	/*3C*/ UInt32					refHandle3C;			// same as above
	/*40*/ UInt32					unk40;
	/*44*/ UInt32					unk44;
	/*48*/ DialogBranchArrayT		unk48;
	/*54*/ DialogBranchArrayT		unk54;
	/*60*/ UInt8					unk60;
	/*61*/ UInt8					unk61;
	/*62*/ UInt8					unk62;
	/*63*/ UInt8					unk63;
	/*64*/ UInt8					unk64;
	/*65*/ UInt8					unk65;
	/*66*/ UInt8					unk66;
	/*67*/ UInt8					unk67;
	/*68*/ UInt8					unk68;
	/*69*/ UInt8					unk69;
	/*6A*/ UInt8					unk6A;					// init to 1
	/*6B*/ UInt8					unk6B;
	/*6C*/ TopicArrayT				unk6C;

	// methods
	virtual void*					Dtor(bool FreeMemory = true) = 0;

	bool							InitiateDialog(TESObjectREFR* Speaker, bool Arg2 = false, TESTopicInfo* Topic = NULL, bool Arg4 = false);
									// initiates dialog with the player character

	static MenuTopicManager*		GetSingleton(void);
};
STATIC_ASSERT(offsetof(MenuTopicManager, topicManagerCS) == 0x20);
STATIC_ASSERT(offsetof(MenuTopicManager, unk54) == 0x54);
STATIC_ASSERT(sizeof(MenuTopicManager) == 0x78);

// arbitrary class to wrap UI related functions
class UIUtils
{
public:
	static void						QueueMessage(const char* Message, UInt32 Arg2 = 0, bool Arg3 = true);
};

_DeclareMemHdlr(TESTopicInfoGetAssetPath, "sneakly swaps file paths to missing voice assets");
_DeclareMemHdlr(ForceSubtitlesMark1, "various shenanigans to get the engine to actually use the 'Force Subtitle' flag in dialog infos");
_DeclareMemHdlr(ForceSubtitlesMark2, "");
_DeclareMemHdlr(ForceSubtitlesMark3, "");
_DeclareMemHdlr(ForceSubtitlesMark4, "");
_DeclareMemHdlr(MainLoopTick, "tick tock cuckoo clock");

void BollocksBollocksBollocks(void);

