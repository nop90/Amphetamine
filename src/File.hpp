#ifndef __AMP_FILES__
#define __AMP_FILES__

#include <stdio.h>
#include "System.hpp"

// Basic types for file input/output. Size of variable is very important
typedef unsigned char Var1Byte;
typedef short Var2Bytes;
typedef long Var4Bytes ;
typedef double Var8Bytes;

/* Macros used to convert big endian to little endian and back. Data in the
	Amphetamine data files is stored in big endian (PowerPC)
*/

#define	SWAP_SHORT(x)	((x << 8) | (((unsigned short)x) >> 8))
#define	SWAP_LONG(x)	(				\
						(x << 24) |		\
						(((unsigned long)x) >> 24) |	\
						((x & 0x0000FF00) << 8) |		\
						((x & 0x00FF0000) >> 8)			\
						)

#define	SWAP_PUT_SHORT(x)	((x) = (x << 8) | (((unsigned short)x) >> 8))
#define	SWAP_PUT_LONG(x)	((x) = (x << 24) | (((unsigned long)x) >> 24) | \
						((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8))

#define	TRANS_NUM(x)	(		\
						(sizeof(x) == sizeof(short)) ? SWAP_SHORT(x) :	\
						(sizeof(x) == sizeof(long)) ? SWAP_LONG(x) : (x)\
						)

#ifndef __BIG_ENDIAN__

#define	TRANS_PUT_NUM(x)	((sizeof(x) == sizeof(short)) ? SWAP_PUT_SHORT(x) :	\
							 (sizeof(x) == sizeof(long)) ? SWAP_PUT_LONG(x) : (x))
#define TRANS_PUT_NUM2(x)   x

#else

#define	TRANS_PUT_NUM(x)	x

#define	TRANS_PUT_NUM2(x)	((sizeof(x) == sizeof(short)) ? SWAP_PUT_SHORT(x) :	\
						     (sizeof(x) == sizeof(long)) ? SWAP_PUT_LONG(x) : (x))

#endif


class	CFile {
protected:
	FILE		*fileRef;
	short		levelNumber;

	boolVar	OpenDataFile(char *name);
	void	SetFilePos(long offset);
	long	ReadData(void *buffer, long size);
	void	CloseDataFile();

public:

	CFile();
	~CFile();
};


// Datastructure of level element
struct tLevelElement {
	Var2Bytes	kind;   // kWall = 0, kBackground = 1
	Var2Bytes	iconID;
	Var2Bytes	light;
	Var2Bytes	refNum;	// LoByte: Platform switch, HiByte: Light Index
	Var2Bytes	bitData;
	Var2Bytes	monsterRef;
	Var2Bytes	itemRef;
};

#endif	
