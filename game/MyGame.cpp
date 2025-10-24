#include "stdafx.h"
#include "MyGame.h"

#pragma warning (disable: 4244)

bool PathFind(vector<NODE>& graph, int nStart, int nGoal, vector<int>& path)
{
	// create a list of open nodes
	vector<int> open;

	// mark all nodes in the graph as unvisited
	for (vector<NODE>::iterator it = graph.begin(); it != graph.end(); ++it) 
	{
		it->open = false;
		it->closed = false;
	}

	// open the start node
	graph[nStart].open = true;
	graph[nStart].costSoFar = 0;
	graph[nStart].nConnection = -1;

	open.push_back(nStart);

	while (open.size() > 0)
	{
		
		// Select the current node. It will be the node in the list of open nodes with the smallest costSoFar
		/*int currentNode = open.front();
		if (open.size() > 1)
		{
			for (vector<int>::iterator it = open.begin(); it != open.end(); ++it)
			{
				if (graph[currentNode].costSoFar > graph[it]->costSoFar  )
				{
					currentNode = it;
				}
			}
		}*/
		list<int>::iterator iCurrent = min_element(open.begin(), open.end(), [graph](int i, int j) -> bool 
			{
				return graph[i].costSoFar < graph[j].costSoFar;
			});
		int currentNode = *iCurrent;

		// Found the goal node?
		if (currentNode == nGoal)
			break;

		for each (CONNECTION connection in graph[currentNode].conlist)
		{
			int endNode = connection.nEnd;
			int newCostSoFar = graph[currentNode].costSoFar + connection.cost;

			// for open nodes, ignore if the current route worse then the old one
			if (graph[endNode].open && graph[endNode].costSoFar <= newCostSoFar)
				continue;

			// Wow, we've found a better route!
			graph[endNode].costSoFar = newCostSoFar;
			graph[endNode].nConnection = currentNode;

			// if unvisited yet, add to the open list
			if (!graph[endNode].open)
			{
				graph[endNode].open = true;
				open.push_back(endNode);
			}
		}

		// We can now close the current node...
		graph[currentNode].closed = true;
		//remove the current node from the open nodes list
		open.erase(iCurrent);
	}

	// Collect the path from the generated graph data
	if (open.size() <= 0)
		return false;  // path not found!

	int i = nGoal;
	while (graph[i].nConnection >= 0)
	{
		path.push_back(i);
		i = graph[i].nConnection;
	}
	path.push_back(i);

	reverse(path.begin(), path.end());
	return true;
}


float Coords[][2] = 
{
	{ 352, 160 }, { 480, 160 }, { 608, 288 }, { 608, 96 }, { 992, 288 }, { 912, 96 }, { 992, 160 }, { 1184, 160 }, { 992, 480 }, { 1184, 480 }, { 1184, 672 }, { 800, 672 },
	{ 736, 480 }, { 544, 480 }, { 512, 672 }, { 96, 672 }, { 96, 416 }, { 352, 416 }
};

int Connections[][2] =
{
	{ 0, 1 }, { 0, 17 }, { 1, 2 }, { 1, 3 }, { 2, 3 }, { 2, 4 }, { 3, 5 }, { 4, 5 }, { 4, 6 }, { 4, 8 }, { 5, 6 }, { 6, 7 }, { 7, 9 }, { 8, 9 }, { 9, 10 },{ 10, 11 }, { 11, 12 },
	{ 11, 14 }, { 12, 13 }, { 13, 14 }, { 14, 15 }, { 15, 16 }, { 16, 17 }
};

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
	m_npc(544, 96, 64, 64, 0),
	m_pumpkin(-100, -100, "Pumpkin22.png", CColor::White(), 0)
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
	for (NODE n : m_graph)
		for (CONNECTION c : n.conlist)
			g->DrawLine(n.pos, m_graph[c.nEnd].pos, CColor::Black());
	m_nodes.for_each(&CSprite::Draw, g);
	m_tiles.for_each(&CSprite::Draw, g);
	m_pumpkin.Draw(g);
	m_npc.Draw(g);
}

/////////////////////////////////////////////////////
// Game Life Cycle

// one time initialisation
void CMyGame::OnInitialize()
{
	// Create Nodes

	// create graph structure - nodes

	for (float* coord : Coords)
	{
		m_graph.push_back(NODE{ CVector(coord[0], coord[1]) });
	}

	// create graph structure - connections

	for (int* conn : Connections)
	{
		int ind1 = conn[0];
		int ind2 = conn[1];
		NODE& node1 = m_graph[ind1];
		NODE& node2 = m_graph[ind2];
		float dist = Distance(node1.pos, node2.pos);
		node1.conlist.push_back(CONNECTION{ ind2, dist });
		node2.conlist.push_back(CONNECTION{ ind1, dist });
	}

	int i = 0;
	for (NODE n : m_graph)
	{
		stringstream s;
		s << i++;
		m_nodes.push_back(new CSpriteOval(n.pos, 12, CColor::White(), CColor::Black(), 0));
		m_nodes.push_back(new CSpriteText(n.pos, "arial.ttf", 14, s.str(), CColor::Black(), 0));
	}

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
	if (m_tileLayout[y / 64][x / 64] != ' ')
		return;	// cannot go into a wall!

	// find the first node: the closest to the NPC
	vector<NODE>::iterator iFirst =
		min_element(m_graph.begin(), m_graph.end(), [this](NODE& n1, NODE& n2) -> bool {
		return Distance(n1.pos, m_npc.GetPos()) < Distance(n2.pos, m_npc.GetPos());
			});
	int nFirst = iFirst - m_graph.begin();

	// find the last node: the closest to the destination
	vector<NODE>::iterator iLast =
		min_element(m_graph.begin(), m_graph.end(), [v](NODE& n1, NODE& n2) -> bool {
		return Distance(n1.pos, v) < Distance(n2.pos, v);
			});
	int nLast = iLast - m_graph.begin();

	// remove the current way points and reset the NPC
	if (!m_waypoints.empty())
	{
		m_waypoints.clear();
		m_npc.SetVelocity(0, 0);
	}

	// call the path finding algorithm to complete the waypoints
	vector<int> path;
	if (PathFind(m_graph, nFirst, nLast, path))
	{
		for (int i : path)
			m_waypoints.push_back(m_graph[i].pos);
		m_waypoints.push_back(v);
	}

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
