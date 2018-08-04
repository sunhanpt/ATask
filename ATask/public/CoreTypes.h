#ifndef __CORETYPES_H__
#define __CORETYPES_H__

using uint32		= unsigned int;
using int32			= int;
using uint64		= unsigned __int64;
using int64			= __int64;
using uint8			= unsigned char;
using int8			= char;
using int16			= short int;
using uint16		= unsigned short int;

template<typename T32BITS, typename T64BITS, int PointerSize>
struct SelectIntPointerType
{
	// nothing here are is it an error if the partial specializations fail
};

template<typename T32BITS, typename T64BITS>
struct SelectIntPointerType<T32BITS, T64BITS, 8>
{
	typedef T64BITS TIntPointer; // select the 64 bit type
};

template<typename T32BITS, typename T64BITS>
struct SelectIntPointerType<T32BITS, T64BITS, 4>
{
	typedef T32BITS TIntPointer; // select the 32 bit type
};

typedef SelectIntPointerType<uint32, uint64, sizeof(void*)>::TIntPointer UPTRINT;
typedef SelectIntPointerType<int32, int64, sizeof(void*)>::TIntPointer PTRINT;

#endif
