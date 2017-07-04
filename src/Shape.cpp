#include "Shape.hpp"
#include "Appl.hpp"
#include "Clut.hpp"
#include <string.h>
#include <memory.h>

extern CApplication *gApplication;
extern CSystem	*gSystem;
extern CClutManager	*gClutManager;
extern tConstValues	*gConst;
extern tConfigData	*gConfigData;


CShape::CShape(unsigned char *bitmap, short descr[], long width)
{
	this->id = descr[0];
	this->dx = descr[3];
	this->dy = descr[4];
}

CShape::~CShape()
{
	delete [] shapeForward;
}

unsigned char	*CShape::AllowPixelAccess(short &ddx, short &ddy)
{
	ddx = dx; ddy = dy;
	return shapeForward;
}

short	CShape::RenderShape(short x, short y, tRect *clipRect, short modus, short percentage, CGraphicSurface *surface)
{
	return dx;
}


#define kEndShapeToken					0L				// the end of shape maker
#define kLineStartToken					1L				// the line start marker
#define kDrawPixelsToken				2L				// the draw run marker
#define kSkipPixelsToken				3L				// the skip pixels marker
#define kClearColorIndex				0				// the index of the color defined as clear (in this case, white)
#define	kFalse							0
#define kTrue							1

CMaskedShape::CMaskedShape(unsigned char *bitmap, short descr[], long width) : CShape(bitmap, descr, width)
{
	shapeForward = new unsigned char [ 8 * dy * dx + 4 * dy + 4 + 8];
	Encode(shapeForward, bitmap, descr, width);
	
	shapeBackward = new unsigned char [ 8 * dy * dx + 4 * dy + 4 + 8];
	Encode(shapeBackward, bitmap, descr, width);
}

/*  Allow unaligned memory access if the architecture supports it (like IA-32)
 */
#ifdef __OPT_MEM_ACCESS__

#define GET_LONG(adr)		*(unsigned long *)adr
#define SET_LONG(adr, l)	*(unsigned long *)adr = l

#else

#define GET_LONG(adr)	((long)((unsigned char *)(adr))[0] << 24) + ((long)((unsigned char *)(adr))[1] << 16) + ((long)((unsigned char *)(adr))[2] << 8) + ((long)((unsigned char *)(adr))[3])
#define SET_LONG(adr, l)	((unsigned char *)adr)[0] = (unsigned char)((l) >> 24); \
	                        ((unsigned char *)adr)[1] = (unsigned char)((l) >> 16); \
	                        ((unsigned char *)adr)[2] = (unsigned char)((l) >> 8); \
	                        ((unsigned char *)adr)[3] = (unsigned char)(l)

#endif


void	CMaskedShape::Encode(unsigned char *shape, unsigned char *bitmap, short descr[], long width) 
{
	unsigned short shapeHeight;				// the height of the shape
	unsigned short shapeWidth;					// the width of the shape
	unsigned char *destPtr;							// the current position in the shape
	unsigned char *srcPtr;								// the current position in the souce graphic data
	unsigned char *baseAddr;						// the base address of the source pixmap
	unsigned long rowBytes;							// the row bytes of the source pixmap
	unsigned char *rowStart;						// the start of the current row in the pixmap
	unsigned long yCounter;								// a counter to scan the shape vertically
	unsigned long xCounter;								// a counter to scan the shape horizontally
	unsigned char drawRunFlag;					// are we in a draw pixels run?
	unsigned char skipRunFlag;					// are we in a skip pixels run?
	unsigned char *lineStartPtr;				// where is the line start token for this line
	unsigned char *runTokenPtr;				// where is the token for the current run
	unsigned long runCounter;						// how long is the current run? 
	long	srcStep = (shape == shapeForward ? 1 : -1);

	// determine the width and height of the shape (we use these values a lot)
	shapeHeight = dy;
	shapeWidth = dx;
	
	// create a handle big enough for the worst case encoding
	// ( 8 bytes/pixel + 4 bytes/row + 4 bytes/shape (end token) + 8 bytes/shape (rect) )

	
	// lock the handle and get the pointer
	destPtr = shape;
	
	// store the shape rect
	((short *)destPtr)[0] = 0;
	((short *)destPtr)[1] = 0;
	((short *)destPtr)[2] = dx;
	((short *)destPtr)[3] = dy;
	destPtr += 8;

	// get the location of the source data
	baseAddr = bitmap;
	rowBytes = width;
	rowStart = baseAddr + rowBytes * descr[2] + descr[1] + (shape == shapeForward ? 0 : dx);
	
	// scan the shape row by row
	for( yCounter = 0; yCounter < shapeHeight; yCounter++ )
		{
			// store the location of this line start
			lineStartPtr = destPtr;
			destPtr += sizeof( unsigned long );
			
			// at the beginning of each row we are not in any run
			drawRunFlag = kFalse;
			skipRunFlag = kFalse;
			
			// move to the start of the row
			srcPtr = rowStart;
			
			// scan each row of the shape
			for( xCounter = 0; xCounter < shapeWidth; xCounter++ ) {
					// is this pixel clear?
					if ( *srcPtr == kClearColorIndex ) {
							// are we in a draw run?
							if ( drawRunFlag ) {
									// end the draw run
									drawRunFlag = kFalse;
									
									// create the draw token
								    SET_LONG(runTokenPtr,  ( kDrawPixelsToken << 24 ) + runCounter);
									
									// pad to a mulitple of four
									SET_LONG(destPtr ,0L);
									destPtr += ( ( runCounter & 3L ) == 0 ) ? 0 : ( 4 - ( runCounter & 3L ) );
								}
							
							// are we in a skip run
							if ( skipRunFlag ) {
									// continue it
									runCounter++;
								} else {
									// start one
									skipRunFlag = kTrue;
									runCounter = 1;
								}
						} else {
							// are we in a skip run
							if ( skipRunFlag ) {
									// end the skip run
									skipRunFlag = kFalse;
									
									// create the skip token
									//*( ( unsigned long * )destPtr ) = ( kSkipPixelsToken << 24 ) + runCounter;
									SET_LONG(destPtr, ( kSkipPixelsToken << 24 ) + runCounter);
									destPtr += sizeof( unsigned long );
								}
							
							// are we in a draw run
							if ( drawRunFlag ) {
									// continue it
									runCounter++;
									
									// copy the pixel
									*destPtr = *srcPtr;

									destPtr++;
								} else {
									// start one
									drawRunFlag = kTrue;
									runCounter = 1;
									
									// save the location of the token (so we can fill it in later)
									runTokenPtr = destPtr;
									destPtr += sizeof( unsigned long );
									
									// copy the pixel
									*destPtr = *srcPtr;

									destPtr++;
								}
						}
					
					// move to the next byte
					srcPtr += srcStep;
				}
			
			// are we in a draw run
			if( drawRunFlag )
				{
					// end the draw run
					drawRunFlag = kFalse;
					
					// create the draw token
					SET_LONG(runTokenPtr, ( kDrawPixelsToken << 24 ) + runCounter);
					
					// pad to a mulitple of four
					SET_LONG(destPtr, 0L);
					destPtr += ( ( runCounter & 3L ) == 0 ) ? 0 : ( 4 - ( runCounter & 3L ) );
				}
			
			// create the line start token
			SET_LONG(lineStartPtr, ( kLineStartToken << 24 ) + ( destPtr - ( lineStartPtr + 4 ) ));
			
			// move the row start to the next row
			rowStart += rowBytes;
		}
	
	// create the end of shape token
	SET_LONG(destPtr, kEndShapeToken << 24);
	destPtr += sizeof( unsigned long );
	
	// Resize the handle to match the real size of the shape
	//SetHandleSize( shapeHandle, destPtr - ( unsigned char * )( *shapeHandle ) );
}



CMaskedShape::~CMaskedShape()
{
	delete [] shapeBackward;
}

short	CMaskedShape::RenderShape(short x, short y, tRect *clipRect, short modus, short percentage, CGraphicSurface *surface)
{
	if (clipRect->left > x || clipRect->top > y || clipRect->right < x + dx || clipRect->bottom < y + dy)
		RenderShapeClipped(x, y, clipRect, modus, percentage, surface);
	else RenderShapeUnclipped(x, y, modus, percentage, surface);

	return dx;
}

void	CMaskedShape::RenderShapeClipped(short x, short y, tRect *sClipRect, short modus, short percentage, CGraphicSurface *surface)
{
	tRect clipRect;		// the rect that defines the clipped shape
	short	pitch;
	unsigned char *rowStart;						// the pointer to the start of this row
	unsigned char *srcPtr;								// the current position in the sprite data
	unsigned char *destPtr;							// the current position in the destination pixmap
	long miscCounter;					// a counter for various purposes
	long extraCounter;				// a counter for right clippling purposes ( how much extra was there? )
	unsigned long tokenOp;									// the op code from the token
	unsigned long tokenData;							// the data from the token
	unsigned char exitFlag;								// should we exit from the loop?
	long yCount;									// how many lines down in the shape are we?
	long xCount;					// where are we in this line?
	unsigned char *shape = (modus & kShapemodusBackwardFlag ? shapeBackward : shapeForward);
	
	modus &= ~kShapemodusBackwardFlag;	// clearing the direction flag

	
	// create a clipped rect in the coordinates of the sprite
	clipRect.left = x < sClipRect->left ? sClipRect->left - x : 0;
	clipRect.right = x + dx > sClipRect->right ? sClipRect->right - x : dx;
	clipRect.top = y < sClipRect->top ? sClipRect->top - y : 0;
	clipRect.bottom = y + dy > sClipRect->bottom ? sClipRect->bottom - y : dy;

	// set up the counters
	yCount = 0;
	xCount = 0;
	// determine characteristics about the pixmap

	rowStart = surface->GetSurfacePtr(&pitch);
	rowStart += y * pitch + x;

	// move to the right place in the shape ( just past the size rect )
	srcPtr = shape + 8;

	// loop until we are done
	exitFlag = kFalse;
	while( !exitFlag )
		{
			// get a token
			tokenOp = GET_LONG(srcPtr ) >> 24;
			tokenData = GET_LONG(srcPtr ) & 0x00ffffff;
			srcPtr += sizeof( unsigned long );
			
			// depending on the token
			switch( tokenOp )
				{
					case kDrawPixelsToken:
						miscCounter = tokenData;
						extraCounter = 0;
						
						// if we need to, clip to the left
						if( xCount < clipRect.left )
							{
								// if this run does not appear at all, don't draw it
								if ( miscCounter < clipRect.left - xCount )
									{
										destPtr += miscCounter;
										srcPtr += miscCounter;
										srcPtr += ( ( tokenData & 3L ) == 0 ) ? 0 : ( 4 - ( tokenData & 3L ) );
										xCount += miscCounter;
										break;
									}
								else
									{
										// if it does, skip to where we can draw
										miscCounter -= clipRect.left - xCount;
										destPtr += (clipRect.left - xCount);
										srcPtr += (clipRect.left - xCount);
										xCount += clipRect.left - xCount;
									}
							}
						
						// if we need to, clip to the right
						if ( xCount + miscCounter > clipRect.right )
							{
								// if this run does not appear at all, skip it
								if ( xCount > clipRect.right )
									{
										destPtr += miscCounter;
										srcPtr += miscCounter;
										srcPtr += ( ( tokenData & 3L ) == 0 ) ? 0 : ( 4 - ( tokenData & 3L ) );
										xCount += miscCounter;
										break;
									}
								else
									{
										// if it does, setup to draw what we can
										extraCounter = miscCounter;
										miscCounter -= ( xCount + miscCounter ) - clipRect.right;
										extraCounter -= miscCounter;
									}
							}
						
						// adjust xCount for the run
						xCount += miscCounter;
						
						if (gConfigData->disableShapeModes) {
							while (miscCounter)
								{
									*(unsigned char *)destPtr = *(unsigned char *)srcPtr; 
									destPtr += sizeof( unsigned char  );
									srcPtr += sizeof( unsigned char  );
									miscCounter -= sizeof( unsigned char );
								}
						}else{
							while (miscCounter)
								{
									gClutManager->SetPixel((unsigned char *)srcPtr, (unsigned char *)destPtr, modus, percentage); 
									destPtr += sizeof( unsigned char  );
									srcPtr += sizeof( unsigned char  );
									miscCounter -= sizeof( unsigned char );
								}
						}
						

						// adjust for right clipping
						destPtr += extraCounter;
						srcPtr += extraCounter;
						xCount += extraCounter;
						
						// adjust for the padding
						srcPtr += ( ( tokenData & 3L ) == 0 ) ? 0 : ( 4 - ( tokenData & 3L ) );
						break;
						
					case kSkipPixelsToken:
						destPtr += tokenData;
						xCount += tokenData;
						break;
						
					case kLineStartToken:
						// if this line is above the clip rect, skip to the next line
						if( yCount < clipRect.top )
							{
								srcPtr += tokenData;
							}

						// set up the destination pointer
						destPtr = rowStart;
						rowStart += pitch;
						
						// move the yCounter
						yCount++;
						
						// reset the xCounter
						xCount = 0;
						
						// if we have hit the bottom clip, exit the loop
						if ( yCount > clipRect.bottom )
							{
								exitFlag = kTrue;
							}
						break;
						
					case kEndShapeToken:
						// signal a loop exit
						exitFlag = kTrue;
						break;
						
					default:
						// we should never get here
						break;
				}
		}
	surface->ReleaseSurface();
}



void	CMaskedShape::RenderShapeUnclipped(short x, short y, short modus, short percentage, CGraphicSurface *surface)
{
	short	pitch;
	unsigned char *rowStart;				// the pointer to the start of this row
	unsigned char *srcPtr;						// the current position in the sprite data
	unsigned char *destPtr;					// the current position in the destination pixmap
	long miscCounter;			// a counter for various purposes
	unsigned long tokenOp;							// the op code from the token
	unsigned long tokenData;					// the data from the token
	unsigned char exitFlag;						// should we exit from the loop?
	unsigned char *shape = (modus & kShapemodusBackwardFlag ? shapeBackward : shapeForward);

	modus &= ~kShapemodusBackwardFlag;	// clearing the direction flag

	// determine characteristics about the pixmap
	rowStart = surface->GetSurfacePtr(&pitch);
	rowStart += y * pitch + x;
	
	// move to the right place in the shape ( just past the size rect )
	srcPtr = shape + 8;
	
	// loop until we are done
	exitFlag = kFalse;
	while( !exitFlag )
		{
			// get a token
			tokenOp = GET_LONG(srcPtr) >> 24;
			tokenData = GET_LONG(srcPtr) & 0x00ffffff;
			srcPtr += sizeof( unsigned long );
			
			// depending on the token
			switch( tokenOp )
				{
					case kDrawPixelsToken:
						miscCounter = tokenData;
						

						if (gConfigData->disableShapeModes) {
							while (miscCounter)
								{
									*(unsigned char *)destPtr = *(unsigned char *)srcPtr; 
									destPtr += sizeof( unsigned char  );
									srcPtr += sizeof( unsigned char  );
									miscCounter -= sizeof( unsigned char );
								}
						}else{	
							while (miscCounter)
								{
									gClutManager->SetPixel((unsigned char *)srcPtr, (unsigned char *)destPtr, modus, percentage); 
									destPtr += sizeof( unsigned char  );
									srcPtr += sizeof( unsigned char  );
									miscCounter -= sizeof( unsigned char );
								}
						}
						

						// adjust for the padding
						srcPtr += ( ( tokenData & 3L ) == 0 ) ? 0 : ( 4 - ( tokenData & 3L ) );
						break;
						
					case kSkipPixelsToken:
						destPtr += tokenData;
						break;
						
					case kLineStartToken:
						// set up the destination pointer
						destPtr = rowStart;
						rowStart += pitch;
						break;
						
					case kEndShapeToken:
						// signal a loop exit
						exitFlag = kTrue;
						break;
						
					default:
						// we should never get here
						break;
				}
		}

	surface->ReleaseSurface();
}




CTexture::CTexture(unsigned char *bitmap, short descr[], long width, short light) : CShape(bitmap, descr, width)
{
	short	j, k;
	unsigned char	*shapePtr;
	short	size = dx * dy;

	shapeForward = new unsigned char[size];

	bitmap = (unsigned char *)((unsigned long)bitmap + descr[2] * width + descr[1]);
	shapePtr = shapeForward;

	for (j = 0; j < dy; j ++) {
		for (k = 0; k < dx; k ++) {
			gClutManager->SetPixel(&(bitmap[k]), &(shapePtr[k]), kShapemodusNormal, light);
		}
		bitmap += width;
		shapePtr += dx;
	}
}


CTexture::~CTexture()
{
}


short	CTexture::RenderShape(short x, short y, tRect *clipRect, short unused, short unused2, CGraphicSurface *surface)
{
	unsigned char	*startDestPtr, *startSourcePtr, *tmpSourcePtr, *tmpDestPtr;
	short	pitch, startX, startY, clipWidth, clipHeight, tmpWidth;

	startDestPtr = surface->GetSurfacePtr(&pitch);
	startDestPtr += MAX(y, clipRect->top) * pitch + MAX(x, clipRect->left);

	startY = clipRect->top > y ? clipRect->top - y : 0;
	startX = clipRect->left > x ? clipRect->left - x : 0;
	startSourcePtr = shapeForward + startY * dx + startX;

	clipWidth = (clipRect->right < x + dx ? clipRect->right - x : dx) - startX;
	clipHeight = (clipRect->bottom < y + dy ? clipRect->bottom - y : dy) - startY;
	
	if (clipWidth > 0 && clipHeight > 0) {
		for (short j = 0; j < clipHeight; j ++) {
			memcpy(startDestPtr, startSourcePtr, clipWidth);
			startSourcePtr += dx;
			startDestPtr += pitch;
		}
	}
	surface->ReleaseSurface();

	return dx;
}


CBackground::CBackground(unsigned char *bitmap, short descr[], long width) : CShape(bitmap, descr, width)
{
	shapeForward = bitmap;

	// For background shapes, descr[1] and descr[2] are the width and height of the background picture
	shapeWidth = descr[1];
	shapeHeight = descr[2];
}

CBackground::~CBackground()
{}

short	CBackground::RenderShape(short x, short y, tRect *clipRect, short bkgndCoordX, short unused2, CGraphicSurface *surface)
{
	unsigned char	*startDestPtr, *startSourcePtr, *tmpSourcePtr, *tmpDestPtr;
	short	pitch, startX, startY, clipWidth, clipHeight, tmpWidth;
	short	bkgndPictCoordX = bkgndCoordX % shapeWidth;
	short	n;

	startDestPtr = surface->GetSurfacePtr(&pitch);
	startDestPtr += MAX(y, clipRect->top) * pitch + MAX(x, clipRect->left);

	startY = MAX(clipRect->top, y);
	startX = (bkgndPictCoordX + MAX(x, clipRect->left)) % shapeWidth;
	startSourcePtr = shapeForward + MAX(y, clipRect->top) * shapeWidth;

	clipWidth = (clipRect->right < x + dx ? clipRect->right : x + dx) - MAX(x, clipRect->left);
	clipHeight = (clipRect->bottom < y + dy ? clipRect->bottom : y + dy) - startY;

	for (short j = 0; j < clipHeight; j ++) {
		tmpWidth = clipWidth;
		tmpDestPtr = startDestPtr;

		if (startX + tmpWidth >= shapeWidth) {
			for (n = 0; n < tmpWidth; n ++) {
				startDestPtr[n] = startSourcePtr[(startX + n) % shapeWidth];
			}
		}else{
			tmpSourcePtr = startSourcePtr + startX;
			memcpy(tmpDestPtr, tmpSourcePtr, tmpWidth);					
		}
		startSourcePtr += shapeWidth;
		startDestPtr += pitch;
	}

	surface->ReleaseSurface();

	return dx;
}
