#pragma once
#include "Sprite.h"

class CGuard : public CSprite
{
private:
	
public:
	bool chasing = false;
	list<CVector> m_waypoints;
	list<CVector> patrolPoints;
	CGuard() : CSprite() {};
	CGuard(const CGuard& m) : CSprite(m) {};
	CGuard(float x, float y, float w, float h, Uint32 time) : CSprite(x, y, w, h, time) {};
	CGuard(CVector v, float w, float h, Uint32 time) : CSprite(v, w, h, time) {};
	void PatrolPointFill();
	void PatrolPointCycling();
};
