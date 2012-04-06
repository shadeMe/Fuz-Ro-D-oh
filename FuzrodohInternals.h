#pragma once

#include <string>

#include "skse/PluginAPI.h"
#include "skse/skse_version.h"
#include "skse/GameTypes.h"

#include "[Libraries]\SME Sundries\SME_Prefix.h"
#include "[Libraries]\SME Sundries\MemoryHandler.h"

using namespace SME;
using namespace SME::MemoryHandler;

extern IDebugLog		gLog;
extern PluginHandle		g_pluginHandle;

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
		///*00*/ void**			vtbl;
		 
		// methods
		virtual void*			Dtor(bool FreeMemory = true) = 0;
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
		/*0C*/ UInt32				unk0C;		// flags, init to 0x10000 or 0x200
		/*10*/ UInt8				unk10;
		/*11*/ UInt8				pad11[3];
	};

	// members
	///*00*/ void**				vtbl;

	// methods
	virtual void*				Dtor(bool FreeMemory = true) = 0;
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

_DeclareMemHdlr(TESTopicInfoGetAssetPath, "sneakly swaps file paths to missing voice assets");

void BollocksBollocksBollocks(void);

