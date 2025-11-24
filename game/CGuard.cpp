#pragma once
#include "stdafx.h"
#include "CGuard.h"
#include "MyGame.h"

extern CMyGame game;
/*
void CGuard::SetSeeDestination(bool newSee)
{
	seeDestination = newSee;
}
bool CGuard::GetSeeDestination()
{
	return seeDestination;
}
*/
void CGuard::PatrolPointFill()
{
	for (CVector pathNode : patrolPoints)
	{
		m_waypoints.push_back(pathNode);
	}
}
void CGuard::PatrolPointCycling()
{
	m_waypoints.push_back(patrolPoints.front());
	patrolPoints.push_back(patrolPoints.front());
	patrolPoints.pop_front();
}