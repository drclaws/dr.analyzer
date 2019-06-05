#pragma once

enum GatherType {
	GatherNone = (INT8)0,

	// Connection types
	GatherActivated = (INT8)1,
	GatherDeactivated = (INT8)2,
	GatherStillUp = (INT8)3,

	// Info types
	GatherFileOpen = (INT8)20,
	GatherLibraryOpen = (INT8)40,

	// Error types
	//GatherThreadError = (INT8)100,
	//GatherDetourError = (INT8)101,
};


class GatherInfo {
public:
	GatherInfo();
	~GatherInfo();

	GatherInfo(INT8 type);
	GatherInfo(INT8 type, LPCWSTR name, INT32 nameLength);
	
	INT32 GetSize();
	void* ToMessageFormat();

private:
	INT8 Type;
	INT32 nameLength;
	LPCWSTR name;
};
