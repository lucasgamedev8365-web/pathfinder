#include "stdafx.h"
#include "MyGame.h"

#pragma warning (disable: 4244)
//problems:
//spider sometimes makes unnecessary journey to nearest node
//I worry about performance

Uint32 DistanceSquaredBetweenPoints(Uint32 x1, Uint32 y1, Uint32 x, Uint32 y)
{
	return (x1 - x) * (x1 - x) + (y1 - y) * (y1 - y);
}

bool PathFind(vector<NODE>& graph, int nStart, int nGoal, vector<int>& path)
{
	// create a list of open nodes
	list<int> open;

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
		//int currentNode = open.front();
		/*if (open.size() > 1)
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

bool Intersection(CVector a, CVector b, CVector c, CVector d, float& k1, float& k2)
{
	CVector v1 = b - a;
	CVector v2 = d - c;
	CVector con = c - a;
	float det = v1.m_x * v2.m_y - v1.m_y * v2.m_x;
	if (det != 0)
	{
		k1 = (v2.m_y * con.m_x - v2.m_x * con.m_y) / det;
		k2 = (v1.m_y * con.m_x - v1.m_x * con.m_y) / det;
		return true;
	}
	else
		return false;
}

bool Intersection(CVector a, CVector b, CVector c, CVector d)
{
	float k1, k2;
	if (!Intersection(a, b, c, d, k1, k2))
		return false;
	return k1 >= 0 && k1 <= 1.f && k2 >= 0 && k2 <= 1.f;
}

bool LineOfSightCheck(CVector point1, CVector point2, CSpriteList p_sprites)
{
	bool SeePoint = true;
	// browse through all tiles - if line of sight test shows any tile to obscure the destination, then we have no shortcut
	for (CSprite* pSprite : p_sprites)
	{
		// Check intersection of the "Guard - Player" sight line with both diagonals of the tile.
		// If there is intersection - there is no killer - so, m_pKiller = NULL;

		// Calls the Intersection function twice, once for each diagonal of the tile to test line of sight

		if ((Intersection(pSprite->GetBottomLeft(), pSprite->GetTopRight(), point1, point2))
			|| (Intersection(CVector(pSprite->GetRight(), pSprite->GetBottom()), CVector(pSprite->GetLeft(), pSprite->GetTop()), point1, point2)))
		{
			SeePoint = false;
		}

		if (!SeePoint)
			break;	// small optimisation, if line of sight test already failed, no point to look further
	}
	return SeePoint;
}

void CMyGame :: CreateSound(Uint32 x, Uint32 y, Uint32 radius)
{
	for (CSprite* pNPC : m_guards)
	{
		if (DistanceSquaredBetweenPoints(pNPC->GetX(), pNPC->GetY(), x, y) <= radius * radius)
		{
			((CGuard*)pNPC)->chasing = true;
		}
	}
	CVector v(x, y);	// destination
	// check if the move is legal
	if (m_tileLayout[y / 64][x / 64] != ' ')
		return;	// cannot go into a wall!
	Waypointing(v);
}

void CMyGame :: Waypointing(CVector v) 
{
	vector<NODE> m_visGraph;
	for (CSprite* pNPC : m_guards)
	{
		if (!((CGuard*)pNPC)->m_waypoints.empty())
		{
			if (LineOfSightCheck(((CGuard*)pNPC)->m_waypoints.back(), v, m_tiles))
			{
				((CGuard*)pNPC)->m_waypoints.push_back(v);
			}
		}

		if (!LineOfSightCheck(pNPC->GetPos(), v, m_tiles))
		{
			//compile the nodes in line of sight of the NPC into a new vector
			m_visGraph.clear();
			for (NODE& n : m_graph)
			{
				if (LineOfSightCheck(pNPC->GetPos(), n.pos, m_tiles))
				{
					m_visGraph.push_back(n);
				}
			}
			if (m_visGraph.empty()) // Error prevention where no node being in line of sight such as inside a wall
			{
				for (NODE& n : m_graph)
				{
					m_visGraph.push_back(n);
				}
			}

			// find the first node: the closest to the NPC in line of sight
			vector<NODE>::iterator iFirst =
				min_element(m_visGraph.begin(), m_visGraph.end(), [pNPC](NODE& n1, NODE& n2) -> bool {
				return Distance(n1.pos, pNPC->GetPos()) < Distance(n2.pos, pNPC->GetPos());
					});
			int nFirst = iFirst - m_visGraph.begin();
			// find the equivalent node in the graph with every node
			for (int i = 0; i <= m_graph.size() - 1; i++)
			{
				if (m_graph[i].pos == m_visGraph[nFirst].pos)
				{
					nFirst = i;
					break;
				}
			}

			//find the nodes in line of sight of the destination
			m_visGraph.clear();
			for (NODE& n : m_graph)
			{
				if (LineOfSightCheck(v, n.pos, m_tiles))
				{
					m_visGraph.push_back(n);
				}
			}
			// find the last node: the closest to the destination
			vector<NODE>::iterator iLast =
				min_element(m_visGraph.begin(), m_visGraph.end(), [v](NODE& n1, NODE& n2) -> bool {
				return Distance(n1.pos, v) < Distance(n2.pos, v);
					});
			int nLast = iLast - m_visGraph.begin();
			// find the equivalent node in the graph with every node
			for (int i = 0; i <= m_graph.size() - 1; i++)
			{
				if (m_graph[i].pos == m_visGraph[nLast].pos)
				{
					nLast = i;
					break;
				}
			}

			// remove the current way points and reset the NPC if the old destination point is not in line of sight with the new one
			if ((!((CGuard*)pNPC)->m_waypoints.empty()) && (!LineOfSightCheck(((CGuard*)pNPC)->m_waypoints.back(), v, m_tiles)))
			{
				((CGuard*)pNPC)->m_waypoints.clear();
				pNPC->SetVelocity(0, 0);
			}

			// call the path finding algorithm to complete the waypoints
			vector<int> path;
			if (PathFind(m_graph, nFirst, nLast, path))
			{
				for (int i : path)
				{
					((CGuard*)pNPC)->m_waypoints.push_back(m_graph[i].pos);
					if (i == 1)
					{
						if ((LineOfSightCheck(pNPC->GetPos(), m_graph[i].pos, m_tiles)) && (LineOfSightCheck(pNPC->GetPos(), m_graph[i - 1].pos, m_tiles)))
						{
							((CGuard*)pNPC)->m_waypoints.pop_front();
						}
					}
				}
				((CGuard*)pNPC)->m_waypoints.push_back(v);
			}
		}
		else
		{
			if (!((CGuard*)pNPC)->m_waypoints.empty()) ((CGuard*)pNPC)->m_waypoints.clear();
			((CGuard*)pNPC)->m_waypoints.push_back(v);
		}
	}
}



float Coords[][2] =
{
	{ 352, 160 }, { 544, 160 }, { 608, 288 }, { 608, 96 }, { 992, 288 }, { 912, 96 }, { 992, 160 }, { 1184, 160 }, { 992, 480 }, { 1184, 480 }, { 1184, 672 }, { 820, 672 },
	{ 730, 480 }, { 544, 480 }, { 544, 672 }, { 96, 672 }, { 96, 480 }, { 352, 480 }, { 352, 416 }, {352, 672}, { 730, 672 }, { 820, 480 }, { 352, 96 }, { 96, 96 },  {96, 232 }, {352, 232 }
};

int Connections[][2] =
{
	{ 0, 1 }, { 1, 2 }, { 1, 3 }, { 2, 3 }, { 2, 4 }, { 3, 5 }, { 4, 5 }, { 4, 6 }, { 4, 8 }, { 5, 6 }, { 6, 7 }, { 7, 9 }, { 8, 9 }, { 9, 10 },{ 10, 11 }, { 11, 12 }
	, { 12, 13 }, { 13, 14 }, { 15, 16 }, { 16, 17 }, {13, 17}, {8, 12},{16, 18 }, { 17, 18 }, {14, 17}, {17, 19}, {13,19}, {11, 20}, {11, 21}, {12, 20}, {12, 21},
	{20,21}, {21,8}, {14,20}, {0, 22}, {0, 23}, {22, 23}, {0, 24}, {22,24}, { 23, 24 }, {25,0}, {25, 18}, {25, 22}, {25, 23}, {24, 25}, {14, 19}, {15, 19}
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
	"X                  X",
	"X XXX    XX  XXXXX X",
	"X XXX    XX  XXXXX X",
	"X                  X",
	"XXXXXXXXXXXXXXXXXXXX",
};
//m_player(100, 100, "Pumpkin22.png", CColor::White(), 0)
CMyGame::CMyGame(void) : m_player(100, 100, 24, 36, CColor::Blue(), {0 , 150, 150}, 0)
{
	for (int i = 0; i < 1; i++)
	{
		CGuard* pNPC = new CGuard(CVector(64, 64), 544, 96, 0);
		if (i == 0) pNPC->SetPos(CVector(544, 96));
		pNPC->LoadAnimation("Spider64.png", "walk", CSprite::Sheet(4, 2).Col(0).From(0).To(1));
		pNPC->LoadAnimation("Spider64.png", "idle", CSprite::Sheet(4, 2).Col(2).From(0).To(1));
		pNPC->SetAnimation("idle", 4);
		m_guards.push_back(pNPC);

	}
}

CMyGame::~CMyGame(void)
{
}

void CMyGame::GuardControl()
{
	for (CSprite* pNPC : m_guards)
	{
		if ((!((CGuard*)pNPC)->m_waypoints.empty()) && (!(((CGuard*)pNPC)->chasing)))// not chasing
		{
			// NPC: follow the waypoints
			// If NPC not moving, start moving to the first waypoint
			if (pNPC->GetSpeed() < 1)
			{
				pNPC->SetDirection(((CGuard*)pNPC)->m_waypoints.front() - pNPC->GetPosition());
				pNPC->SetAnimation("walk");
				pNPC->SetRotation(pNPC->GetDirection() - 90);
			}
			pNPC->SetSpeed(100); // no line of sight and no chasing
			// Passed the waypoint?
			CVector v = ((CGuard*)pNPC)->m_waypoints.front() - pNPC->GetPosition();
			if (Dot(pNPC->GetVelocity(), v) < 0)
			{
				//Next point
				((CGuard*)pNPC)->PatrolPointCycling();
				((CGuard*)pNPC)->m_waypoints.pop_front();
				((CGuard*)pNPC)->chasing = false;
			}
		}
		else if ((!((CGuard*)pNPC)->m_waypoints.empty()) && ((((CGuard*)pNPC)->chasing))) // chasing
		{
			// NPC: follow the waypoints
			// If NPC not moving, start moving to the first waypoint
			if (pNPC->GetSpeed() < 1)
			{
				pNPC->SetDirection(((CGuard*)pNPC)->m_waypoints.front() - pNPC->GetPosition());
				pNPC->SetAnimation("walk");
				pNPC->SetRotation(pNPC->GetDirection() - 90);
			}
			pNPC->SetSpeed(200); // line of sight and chasing
			// Passed the waypoint?
			CVector v = ((CGuard*)pNPC)->m_waypoints.front() - pNPC->GetPosition();
			if (Dot(pNPC->GetVelocity(), v) < 0)
			{
				((CGuard*)pNPC)->m_waypoints.pop_front();
				if (((CGuard*)pNPC)->m_waypoints.empty())
				{
					((CGuard*)pNPC)->chasing = false;
				}
			}
			else if (((LineOfSightCheck(pNPC->GetPos(), ((CGuard*)pNPC)->m_waypoints.back(), m_tiles)) && (((CGuard*)pNPC)->m_waypoints.size() > 1)) && (((CGuard*)pNPC)->chasing))
			{
				CVector lastWayPoint = ((CGuard*)pNPC)->m_waypoints.back();
				((CGuard*)pNPC)->m_waypoints.clear();
				((CGuard*)pNPC)->m_waypoints.push_front(lastWayPoint);
			}
		}
		// if waypoints are empty, return to the last patrol point
		else
		{
			Waypointing(((CGuard*)pNPC)->patrolPoints.front());
			((CGuard*)pNPC)->m_waypoints.pop_back();
			((CGuard*)pNPC)->PatrolPointCycling();
			((CGuard*)pNPC)->chasing = false;
		}
		// player spotted
		if (((LineOfSightCheck(pNPC->GetPos(), m_player.GetPos(), m_tiles)) && (Dot(pNPC->GetVelocity().Normalise(), (m_player.GetPos() - pNPC->GetPos()).Normalise()) > 0.5)) && (DistanceSquaredBetweenPoints(pNPC->GetX(), pNPC->GetY(), m_player.GetX(), m_player.GetY()) <= 250000))
		{
			((CGuard*)pNPC)->chasing = true;
			((CGuard*)pNPC)->m_waypoints.clear();
			((CGuard*)pNPC)->m_waypoints.push_back(m_player.GetPos());
		}
		if (!((CGuard*)pNPC)->m_waypoints.empty())
		{
			pNPC->SetDirection(((CGuard*)pNPC)->m_waypoints.front() - pNPC->GetPosition());
			pNPC->SetRotation(pNPC->GetDirection() - 90);
		}
	}
}

void CMyGame::PlayerControl()
{
	m_player.SetSpeed(150);
	if ((IsKeyDown(SDLK_w)) && (IsKeyDown(SDLK_d)))
	{
		m_player.SetDirection(45);
	}
	else if ((IsKeyDown(SDLK_s)) && (IsKeyDown(SDLK_d)))
	{
		m_player.SetDirection(135);
	}
	else if ((IsKeyDown(SDLK_s)) && (IsKeyDown(SDLK_a)))
	{
		m_player.SetDirection(225);
	}
	else if ((IsKeyDown(SDLK_w)) && (IsKeyDown(SDLK_a)))
	{
		m_player.SetDirection(315);
	}
	else if (IsKeyDown(SDLK_w))
	{
		m_player.SetDirection(0);
	}
	else if (IsKeyDown(SDLK_s))
	{
		m_player.SetDirection(180);
	}
	else if (IsKeyDown(SDLK_d))
	{
		m_player.SetDirection(90);
	}
	else if (IsKeyDown(SDLK_a))
	{
		m_player.SetDirection(270);
	}
	else
	{
		m_player.SetSpeed(0);
	}
	for (CSprite* pWall : m_tiles)
	{
		CVector distVect;
		float timeFrame = (float) GetDeltaTime() / 1000.0f;
		CVector playerV = m_player.GetVelocity() * (timeFrame);
		CVector playerT = pWall->GetPos() - m_player.GetPos();

		// "vertical" check
		CVector normal = CVector(sin(DEG2RAD(pWall->GetRotation())), cos(DEG2RAD(pWall->GetRotation())));
		float Vy = Dot(playerV, normal); // velocity perpendicular component
		if (Vy != 0)
		{
			// perpendicular component 
			if (Vy < 0)
			{
				distVect = playerT + ((((pWall->GetHeight() / 2) + (m_player.GetHeight() / 2))) * normal);
			}
			else
			{
				distVect = playerT - ((((pWall->GetHeight() / 2) + (m_player.GetHeight() / 2))) * normal);
			}
			float f1 = Dot(distVect, normal) / Vy;

			// Parallel component
			float f2 = (Cross(playerT, normal) - (Cross(playerV, normal) * f1)) / (pWall->GetWidth() / 2 + m_player.GetWidth() / 2);

			if (((f1 >= 0 && f1 <= 1) && (f2 >= -1 && f2 <= 1)) && (Vy < 0))
			{
				//player.SetVelocity(Reflect(player.GetVelocity() * RESTITUTION, normal));
				m_player.SetYVelocity(0);
			}
			if (((f1 >= 0 && f1 <= 1) && (f2 >= -1 && f2 <= 1)) && (Vy > 0))
			{
				//player.SetVelocity(Reflect(player.GetVelocity() * RESTITUTION, -1 * normal));
				m_player.SetYVelocity(0);
			}
		}
		// "horizontal" check
		normal = CVector(sin(DEG2RAD(pWall->GetRotation() + 90)), cos(DEG2RAD(pWall->GetRotation() + 90)));
		Vy = Dot(playerV, normal); // velocity perpendicular component
		if (Vy != 0)
		{
			// perpendicular component 
			if (Vy < 0) distVect = ((((pWall->GetWidth() / 2) + (m_player.GetWidth() / 2))) * normal) + playerT;
			else distVect = playerT - ((((pWall->GetWidth() / 2) + (m_player.GetWidth() / 2))) * normal);
			float f1 = Dot(distVect, normal) / Vy;

			// Parallel component
			float f2 = (Cross(playerT, normal) - (Cross(playerV, normal) * f1)) / (pWall->GetHeight() / 2 + m_player.GetHeight() / 2);
			if (((f1 >= 0 && f1 <= 1) && (f2 >= -1 && f2 <= 1)) && ((Vy < 0)))
			{
				m_player.SetXVelocity(0);
				//player.SetVelocity(Reflect(player.GetVelocity() * RESTITUTION, normal));
			}
			else if (((f1 >= 0 && f1 <= 1) && (f2 >= -1 && f2 <= 1)) && ((Vy > 0)))
			{
				m_player.SetXVelocity(0);
				//player.SetVelocity(Reflect(player.GetVelocity() * RESTITUTION, -1 * normal));
			}
		}
	}
}

// Stop movement
					

void CMyGame::OnUpdate()
{
	Uint32 t = GetTime();
	GuardControl();
	PlayerControl();
	for (CSprite* pNPC : m_guards)
	{
		pNPC->Update(t);
	}
	m_player.Update(t);
}

void CMyGame::OnDraw(CGraphics* g)
{
	for (NODE n : m_graph)
		for (CONNECTION c : n.conlist)
			g->DrawLine(n.pos, m_graph[c.nEnd].pos, CColor::Black());
	m_nodes.for_each(&CSprite::Draw, g);
	m_tiles.for_each(&CSprite::Draw, g);
	m_player.Draw(g);
	for (CSprite* pNPC : m_guards)
	{
		pNPC->Draw(g);
		*g << font(28) << color(CColor::Blue()) << xy(10, 570) << "Score: " << num << "nFirst: " << num1;
	}
	//if ()
	
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

	//Guard paths
	for (CSprite* pNPC : m_guards)
	{
		((CGuard*)pNPC)->patrolPoints.push_back(m_graph[1].pos);
		((CGuard*)pNPC)->patrolPoints.push_back(m_graph[2].pos);
		((CGuard*)pNPC)->patrolPoints.push_back(m_graph[4].pos);
		((CGuard*)pNPC)->patrolPoints.push_back(m_graph[5].pos);
		((CGuard*)pNPC)->patrolPoints.push_back(m_graph[3].pos);
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
	CreateSound(x, y, 2000);
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
