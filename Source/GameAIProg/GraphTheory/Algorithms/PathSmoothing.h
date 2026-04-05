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
			
			const auto CurrentEdge{NavPoly.GetEdges()[CurrentNode->GetEdgeIdx()]};

			const FVector2D PathP1{CurrentNode->GetPosition()};
			const FVector2D PathP2{NextNode->GetPosition()};
			
			FVector2D EdgeP1{CurrentEdge.GetP1(NavPoly).X, CurrentEdge.GetP1(NavPoly).Y};
			FVector2D EdgeP2{CurrentEdge.GetP2(NavPoly).X, CurrentEdge.GetP2(NavPoly).Y};

			if (SegmentsIntersect2D(EdgeP1, EdgeP2, PathP1, PathP2))
			{
				if (FVector2D::CrossProduct(PathP2 - PathP1, EdgeP2 - EdgeP2) < 0.0f)
				{
					std::swap(EdgeP1, EdgeP2);
				}
				Portals.emplace_back(EdgeP1, EdgeP2);
			}
		}
		
		Portals.emplace_back(Path.back()->GetPosition(), Path.back()->GetPosition());

        return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		std::vector<FVector2D> Path{};

		if (Portals.empty()) return Path;

		auto ApexPoint{Portals.front().P1};

		FVector2D RightLeg{Portals.front().P1 - ApexPoint};
		FVector2D LeftLeg{Portals.front().P2 - ApexPoint};

		int RightLegIndex{0};
		int LeftLegIndex{0};

		int CurrentPortalIdx = 1;

		Path.emplace_back(ApexPoint);

		//P1 == right point of portal, P2 == left point of portal
		
			//--- RIGHT CHECK ---
			//1. See if moving funnel inwards - RIGHT
			
				//2. See if new line degenerates a line segment - RIGHT
				
					//Leftleg becomes new apex point

					//Calculate new legs (if not the end)


			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT

				//2. See if new line degenerates a line segment - LEFT

					//Rightleg becomes new apex point

					//Calculate new legs (if not the end)


		// Add last path point
		const NavLine &LastPortal = Portals.back();
		FVector2D LastPoint = LastPortal.P1;
		if (Path.empty() || !(Path.back() == LastPoint))
			Path.push_back(LastPoint);

		return Path;
	}
private:

		static bool SegmentsIntersect2D(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D)
		{
			const FVector2D AB = B - A;
			const FVector2D AC = C - A;
			const FVector2D AD = D - A;
			const FVector2D CD = D - C;
			const FVector2D CA = A - C;
			const FVector2D CB = B - C;

			const float D1 = FVector2d::CrossProduct(AB, AC);
			const float D2 = FVector2d::CrossProduct(AB, AD);
			const float D3 = FVector2d::CrossProduct(CD, CA);
			const float D4 = FVector2d::CrossProduct(CD, CB);

			// Proper intersection (straddling)
			if (
				((D1 > 0.f && D2 < 0.f) || (D1 < 0.f && D2 > 0.f)) &&
				((D3 > 0.f && D4 < 0.f) || (D3 < 0.f && D4 > 0.f)))
			{
				return true;
			}

			// Collinear / on-segment checks
			auto OnSegment = [](const FVector2D& P, const FVector2D& Q, const FVector2D& R) -> bool
			{
				// Q lies on segment PR?
				return FMath::Min(P.X, R.X) <= Q.X + KINDA_SMALL_NUMBER &&
					   Q.X <= FMath::Max(P.X, R.X) + KINDA_SMALL_NUMBER &&
					   FMath::Min(P.Y, R.Y) <= Q.Y + KINDA_SMALL_NUMBER &&
					   Q.Y <= FMath::Max(P.Y, R.Y) + KINDA_SMALL_NUMBER;
			};

			const float EPS = KINDA_SMALL_NUMBER;
			if (FMath::IsNearlyZero(D1, EPS) && OnSegment(A, C, B)) return true;
			if (FMath::IsNearlyZero(D2, EPS) && OnSegment(A, D, B)) return true;
			if (FMath::IsNearlyZero(D3, EPS) && OnSegment(C, A, D)) return true;
			if (FMath::IsNearlyZero(D4, EPS) && OnSegment(C, B, D)) return true;

			return false;
		}
		
	SSFA() {}
	~SSFA() {}
};
}
