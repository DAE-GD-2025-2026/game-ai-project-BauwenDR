#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "VectorUtil.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
{
public:
	//=== SSFA Functions ===
	//--- References ---
	//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
	//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, TriPolygon const & NavPoly)
	{
		std::vector<NavLine> Portals{};
		if (Path.empty()) return Portals;

		Portals.emplace_back(Path.front()->GetPosition(), Path.front()->GetPosition());

		for (int Index{0}; Index < Path.size()-1; ++Index)
		{
			const auto CurrentNode{static_cast<NavGraphNode*>(Path[Index])};
			const auto NextNode{static_cast<NavGraphNode*>(Path[Index+1])};

			const int EdgeIndex{CurrentNode->GetEdgeIdx()};
			if (EdgeIndex < 0) continue;
			
			const auto CurrentEdge{NavPoly.GetEdges()[CurrentNode->GetEdgeIdx()]};

			const FVector2D PathP1{CurrentNode->GetPosition()};
			const FVector2D PathP2{NextNode->GetPosition()};
			
			FVector2D EdgeP1{CurrentEdge.GetP1(NavPoly).X, CurrentEdge.GetP1(NavPoly).Y};
			FVector2D EdgeP2{CurrentEdge.GetP2(NavPoly).X, CurrentEdge.GetP2(NavPoly).Y};

				if (FVector2D::CrossProduct(EdgeP2 - EdgeP1, PathP2 - PathP1) > KINDA_SMALL_NUMBER)
				{
					std::swap(EdgeP1, EdgeP2);
				}
				Portals.emplace_back(EdgeP1, EdgeP2);
		}
		
		Portals.emplace_back(Path.back()->GetPosition(), Path.back()->GetPosition());

        return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		std::vector<FVector2D> Path{};

		if (Portals.empty()) return Path;
		if (Portals.size() < 3) return {Portals.front().P1, Portals.back().P1};

		auto ApexPoint{Portals.front().P1};
		
		int CurrentPortalIndex{1};
		int RightLegIndex{1};
		int LeftLegIndex{1};
		
		// P1 == right point of portal, P2 == left point of portal
		Path.emplace_back(ApexPoint);
		
		FVector2D RightLeg{Portals[RightLegIndex].P1 - ApexPoint};
		FVector2D LeftLeg{Portals[LeftLegIndex].P2 - ApexPoint};

		while (CurrentPortalIndex < Portals.size())
		{
			// --- RIGHT CHECK ---
			const FVector2D NewRightLeg{Portals[CurrentPortalIndex].P1 - ApexPoint};

			// 1. See if moving funnel inwards - RIGHT
			if (FVector2D::CrossProduct(NewRightLeg, RightLeg) <= 0.0f)
			{
				// 2. See if new line degenerates a line segment - RIGHT
				if (FVector2D::CrossProduct(NewRightLeg, LeftLeg) < 0.0f)
				{
					// LefLLeg becomes new apex point
					Path.emplace_back(ApexPoint + LeftLeg);
					ApexPoint = Path.back();
					
					CurrentPortalIndex = LeftLegIndex + 1;
					if (CurrentPortalIndex < Portals.size())
					{
						RightLegIndex = CurrentPortalIndex;
						LeftLegIndex = CurrentPortalIndex;
						RightLeg = Portals[RightLegIndex].P1 - ApexPoint;
						LeftLeg = Portals[LeftLegIndex].P2 - ApexPoint;
					}
					continue;
				}
				
				// Calculate new legs (if not the end)
				RightLeg = NewRightLeg;
				RightLegIndex = CurrentPortalIndex;
			}

			// --- LEFT CHECK ---
			const FVector2D NewLeftLeg{Portals[CurrentPortalIndex].P2 - ApexPoint};
			
			// 1. See if moving funnel inwards - LEFT
			if (FVector2D::CrossProduct(NewLeftLeg, LeftLeg) >= 0.0f)
			{
				// 2. See if new line degenerates a line segment - LEFT
				if (FVector2D::CrossProduct(NewLeftLeg, RightLeg) > 0.0f)
				{
					// RightLeg becomes new apex point
					Path.emplace_back(ApexPoint + RightLeg);
					ApexPoint = Path.back();
					
					CurrentPortalIndex = RightLegIndex + 1;
					if (CurrentPortalIndex < Portals.size())
					{
						RightLegIndex = CurrentPortalIndex;
						LeftLegIndex = CurrentPortalIndex;
						RightLeg = Portals[RightLegIndex].P1 - ApexPoint;
						LeftLeg = Portals[LeftLegIndex].P2 - ApexPoint;
					}
					continue;
				}
				
				// Calculate new legs (if not the end)
				LeftLeg = NewLeftLeg;
				LeftLegIndex = CurrentPortalIndex;
			}

			// If neither apex advanced, move on to next portal
			++CurrentPortalIndex;
		}

		// Add last path point
		Path.emplace_back(Portals.back().P1);

		return Path;
	}
private:
	SSFA() {}
	~SSFA() {}
};
}
