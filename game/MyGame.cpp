#include "stdafx.h"
#include "MyGame.h"

#pragma warning (disable: 4244)

char* CMyGame::m_tileLayout[12] =
{
	"XXXXXXXXXXXXXXXXXXXX",
	"X     XX        XX X",
	"X         XXXX     X",
	"X     XX  XXXX  XX X",
	"XXXXX XX        XX X",
	"XXXXX XXXXXXXXX XX X",
	"X     XXXXXXXXX XX X",
	"X  XXXXX     XX    X",
	"X XXXXXX XX  XXXXX X",
	"X XXX    XX  XXXXX X",
	"X                  X",
	"XXXXXXXXXXXXXXXXXXXX",
};

CMyGame::CMyGame(void) :
	m_npc(544, 96, 64, 64, 0)
{
	m_npc.LoadAnimation("Spider64.png", "walk", CSprite::Sheet(4, 2).Col(0).From(0).To(1));
	m_npc.LoadAnimation("Spider64.png", "idle", CSprite::Sheet(4, 2).Col(2).From(0).To(1));
	m_npc.SetAnimation("idle", 4);
}

CMyGame::~CMyGame(void)
{
}

/////////////////////////////////////////////////////
// Per-Frame Callback Funtions (must be implemented!)

void CMyGame::OnUpdate()
{
	Uint32 t = GetTime();

	// NPC: follow the waypoints
	if (!m_waypoints.empty())
	{
		// If NPC not moving, start moving to the first waypoint
		if (m_npc.GetSpeed() < 1)
		{
			m_npc.SetSpeed(500);
			m_npc.SetAnimation("walk");
			m_npc.SetDirection(m_waypoints.front() - m_npc.GetPosition());
			m_npc.SetRotation(m_npc.GetDirection() - 90);
		}

		// Passed the waypoint?
		CVector v = m_waypoints.front() - m_npc.GetPosition();
		if (Dot(m_npc.GetVelocity(), v) < 0)
		{
			// Stop movement
			m_waypoints.pop_front();
			if (m_waypoints.empty())
				m_npc.SetAnimation("idle");
			m_npc.SetVelocity(0, 0);
			m_npc.SetRotation(0);
		}
	}
	
	m_npc.Update(t);
}

void CMyGame::OnDraw(CGraphics* g)
{
	//for (NODE n : m_graph)
	//	for (CONNECTION c : n.conlist)
	//		g->DrawLine(n.pos, m_graph[c.nEnd].pos, CColor::Black());
	m_nodes.for_each(&CSprite::Draw, g);
	m_tiles.for_each(&CSprite::Draw, g);
	m_npc.Draw(g);
}

/////////////////////////////////////////////////////
// Game Life Cycle

// one time initialisation
void CMyGame::OnInitialize()
{
	// Create Nodes

	//int i = 0;
	//for (NODE n : m_graph)
	//{
	//	stringstream s;
	//	s << i++;
	//	m_nodes.push_back(new CSpriteOval(n.pos, 12, CColor::White(), CColor::Black(), 0));
	//	m_nodes.push_back(new CSpriteText(n.pos, "arial.ttf", 14, s.str(), CColor::Black(), 0));
	//}

	// Create Tiles
	for (int y = 0; y < 12; y++)
		for (int x = 0; x < 20; x++)
		{
			if (m_tileLayout[y][x] == ' ')
				continue;

			int nTile = 5;
			if (y > 0 && m_tileLayout[y - 1][x] == ' ') nTile -= 3;
			if (y < 11 && m_tileLayout[y + 1][x] == ' ') nTile += 3;
			if (x > 0 && m_tileLayout[y][x - 1] == ' ') nTile--;
			if (x < 20 && m_tileLayout[y][x + 1] == ' ') nTile++;
			if (nTile == 5 && x > 0 && y > 0 && m_tileLayout[y - 1][x - 1] == ' ') nTile = 14;
			if (nTile == 5 && x < 20 && y > 0 && m_tileLayout[y - 1][x + 1] == ' ') nTile = 13;
			if (nTile == 5 && x > 0 && y < 11 && m_tileLayout[y + 1][x - 1] == ' ') nTile = 11;
			if (nTile == 5 && x < 20 && y < 11 && m_tileLayout[y + 1][x + 1] == ' ') nTile = 10;
			
			nTile--;
			m_tiles.push_back(new CSprite(x * 64.f + 32.f, y * 64.f + 32.f, new CGraphics("tiles.png", 3, 5, nTile % 3, nTile / 3), 0));
		}
}

// called when a new game is requested (e.g. when F2 pressed)
// use this function to prepare a menu or a welcome screen
void CMyGame::OnDisplayMenu()
{
	StartGame();	// exits the menu mode and starts the game mode
}

// called when a new game is started
// as a second phase after a menu or a welcome screen
void CMyGame::OnStartGame()
{
}

// called when a new level started - first call for nLevel = 1
void CMyGame::OnStartLevel(Sint16 nLevel)
{
}

// called when the game is over
void CMyGame::OnGameOver()
{
}

// one time termination code
void CMyGame::OnTerminate()
{
}

/////////////////////////////////////////////////////
// Keyboard Event Handlers

void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	if (sym == SDLK_F4 && (mod & (KMOD_LALT | KMOD_RALT)))
		StopGame();
	if (sym == SDLK_SPACE)
		PauseGame();
	if (sym == SDLK_F2)
		NewGame();
}

void CMyGame::OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode)
{
}


/////////////////////////////////////////////////////
// Mouse Events Handlers

void CMyGame::OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle)
{
}

void CMyGame::OnLButtonDown(Uint16 x, Uint16 y)
{
	CVector v(x, y);	// destination

	// check if the move is legal

	// find the first node: the closest to the NPC

	// find the last node: the closest to the destination

	// remove the current way points and reset the NPC

	// call the path finding algorithm to complete the waypoints
}

void CMyGame::OnLButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonUp(Uint16 x,Uint16 y)
{
}
