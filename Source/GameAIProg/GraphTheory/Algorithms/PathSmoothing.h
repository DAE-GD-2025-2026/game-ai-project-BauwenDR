#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
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
		std::vector<NavLine> Portals;
        if (Path.empty()) return Portals;

        // Helper to get node position
        auto GetPos = [](Node const* n) { return n->GetPosition(); };

        // If path has single node -> degenerate portal from start to end
        if (Path.size() == 1)
        {
            FVector2D p = GetPos(Path[0]);
            Portals.emplace_back(p, p);
            return Portals;
        }

        // For each consecutive pair of nodes, find the shared edge between them.
        for (size_t i = 0; i + 1 < Path.size(); ++i)
        {
            FVector2D a = GetPos(Path[i]);
            FVector2D b = GetPos(Path[i+1]);

            // The nodes were created as midpoints of edges. Find which edge indices match these midpoints.
            // Search polygon edges for matching midpoints (with epsilon).
            const auto &edges = NavPoly.GetEdges();
            auto FindEdgeIndexByMidpoint = [&](FVector2D const &pt)->std::optional<int>
            {
                const float EPS = 1e-4f;
                for (size_t ei = 0; ei < edges.size(); ++ei)
                {
                    FVector e1 = edges[ei].GetP1(NavPoly);
                    FVector e2 = edges[ei].GetP2(NavPoly);
                    FVector2D mid{ (e1.X + e2.X) * 0.5f, (e1.Y + e2.Y) * 0.5f };
                    if (std::fabs(mid.X - pt.X) <= EPS && std::fabs(mid.Y - pt.Y) <= EPS)
                        return static_cast<int>(ei);
                }
                return std::nullopt;
            };

            auto optA = FindEdgeIndexByMidpoint(a);
            auto optB = FindEdgeIndexByMidpoint(b);

            if (!optA || !optB)
            {
                // fallback: construct portal as segment between a and b
                Portals.emplace_back(a, b);
                continue;
            }

            // The shared edge between the triangles adjacent to the path nodes is the edge that is present
            // in both node's triangles. But simpler: find triangle that contains the segment connecting the two nodes.
            // For robustness, consider the polygon edges shared by the two nodes' adjacent triangles.
            // We'll attempt to find an edge that is common to a triangle that has both edge indices.
            bool found = false;
            for (const auto &tri : NavPoly.GetTriangles())
            {
                auto triEdges = tri.GetEdges();
                for (int e = 0; e < 3; ++e)
                {
                    // compute midpoint for this triangle edge
                    auto edge = triEdges[e];
                    FVector e1 = edge.GetP1(NavPoly);
                    FVector e2 = edge.GetP2(NavPoly);
                    FVector2D mid{ (e1.X + e2.X) * 0.5f, (e1.Y + e2.Y) * 0.5f };
                    const float EPS = 1e-4f;
                    if ((std::fabs(mid.X - a.X) <= EPS && std::fabs(mid.Y - a.Y) <= EPS) ||
                        (std::fabs(mid.X - b.X) <= EPS && std::fabs(mid.Y - b.Y) <= EPS))
                    {
                        // This triangle edge corresponds to one of our nodes' edges; use the actual edge endpoints as portal
                        FVector2D p1{ e1.X, e1.Y };
                        FVector2D p2{ e2.X, e2.Y };

                        // Order points so P1 is the right point relative to travel direction (a->b)
                        FVector2D dir{ b.X - a.X, b.Y - a.Y };
                        auto RightOf = [&](FVector2D const &p, FVector2D const &q, FVector2D const &r)
                        {
                            // returns >0 if r is to the left of pq
                            return (q.X - p.X) * (r.Y - p.Y) - (q.Y - p.Y) * (r.X - p.X);
                        };
                        float side = RightOf(a, b, p1);
                        if (side < 0) // p1 is right of direction
                            Portals.emplace_back(p1, p2);
                        else
                            Portals.emplace_back(p2, p1);

                        found = true;
                        break;
                    }
                }
                if (found) break;
            }

            if (!found)
            {
                // fallback - simple segment between the two midpoints, order by right/left relative to dir a->b
                FVector2D dir{ b.X - a.X, b.Y - a.Y };
                auto cross = [&](FVector2D const &u, FVector2D const &v){ return u.X * v.Y - u.Y * v.X; };
                FVector2D rel = { /* use edge endpoints from optA if available */ a.X - b.X, a.Y - b.Y };
                // pick arbitrary ordering
                Portals.emplace_back(a, b);
            }
        }

        // add degenerate portal at goal (last node position)
        FVector2D goal = GetPos(Path.back());
        Portals.emplace_back(goal, goal);

        return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		std::vector<FVector2D> Path{};
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

		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
