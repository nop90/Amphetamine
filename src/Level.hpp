#ifndef __AMP_LEVEL__
#define __AMP_LEVEL__

#include "File.hpp"
#include "Element.hpp"
#include "AmpHead.hpp"
#include "ShapeLd.hpp"
#include "Thing.hpp"

class	CLevel : public CFile {
protected:
	void		LoadBackground(short levNum);
public:	
	CElement	*level[kLevelHeight][kLevelWidth];
	CThing		*player; 
	CThing		*focus;

	
	CLevel(short levelNumber, char *fileName);
	~CLevel();

	void		PaintLevel();
	CElement	*GetElement(double x, double y);
	void		SwitchLight(short lightID);
	CElement	*FindRefnum(short refnum);
	void		WriteLevel(char *fileName);
	short		ReadLevel(char *fileName);
	void		GetSavedGameTitle(char *fileName, char *title);
};


#endif