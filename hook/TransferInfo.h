#pragma once

enum TransferType {
	TransNone = (INT8)0,

	// Connection types
	TransOpen = (INT8)1,
	TransClose = (INT8)2,
	TransStillUp = (INT8)3,

	// Info types
	TransFileOpen = (INT8)20,
	TransFileClose = (INT8)21,
	TransLibraryOpen = (INT8)22,

	// Error types
	TransThreadError = (INT8)100,
	TransDetourError = (INT8)101,
};


class TransferInfo {
public:
	INT8 Type;
	INT32 nameLength;
	LPCWSTR name;

	TransferInfo();
	TransferInfo(INT8 type);
	TransferInfo(INT8 type, LPCWSTR name, INT32 nameLength);

	INT32 GetSize();
};
