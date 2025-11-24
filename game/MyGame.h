#pragma once
#include "CGuard.h"

struct CONNECTION

{

	int nEnd; // index of the destination node

	float cost; // cost of the transition

};

struct NODE

{

	CVector pos; // 2D position of the node

	list<CONNECTION> conlist; // list of connections
	float costSoFar;
	int nConnection;
	bool open, closed;
};

class CMyGame : public CGame
{
	CSpriteRect m_player;
	CSpriteList m_tiles;			// Tiles
	CSpriteList m_nodes;			// Nodes
	CSpriteList m_guards;
	static char *m_tileLayout[12];	// Tiles layout
	float num = 0;
	float num1 = 0;

	vector<NODE> m_graph;

public:
	CMyGame(void);
	~CMyGame(void);
	
	// My own functions
	virtual void Waypointing(CVector v);
	virtual void PlayerControl();
	virtual void GuardControl();
	virtual void CreateSound(Uint32 x, Uint32 y, Uint32 radius);

	// Per-Frame Callback Funtions (must be implemented!)
	virtual void OnUpdate();
	virtual void OnDraw(CGraphics* g);

	// Game Life Cycle
	virtual void OnInitialize();
	virtual void OnDisplayMenu();
	virtual void OnStartGame();
	virtual void OnStartLevel(Sint16 nLevel);
	virtual void OnGameOver();
	virtual void OnTerminate();

	// Keyboard Event Handlers
	virtual void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
	virtual void OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode);

	// Mouse Events Handlers
	virtual void OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle);
	virtual void OnLButtonDown(Uint16 x,Uint16 y);
	virtual void OnLButtonUp(Uint16 x,Uint16 y);
	virtual void OnRButtonDown(Uint16 x,Uint16 y);
	virtual void OnRButtonUp(Uint16 x,Uint16 y);
	virtual void OnMButtonDown(Uint16 x,Uint16 y);
	virtual void OnMButtonUp(Uint16 x,Uint16 y);
};
