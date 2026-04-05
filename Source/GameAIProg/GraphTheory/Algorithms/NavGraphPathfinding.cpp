#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals)
{
	//Create the path to return
	std::vector<FVector2D> FinalPath{};

	//Get the start and
	const auto StartTriangle{pNavGraph->GetNavPolygon()->GetTriangleAtPosition(startPos, false)};
	const auto DestTriangle{pNavGraph->GetNavPolygon()->GetTriangleAtPosition(endPos, false)};

	//We have valid start/end triangles, and they are not the same
	if (StartTriangle == nullptr || DestTriangle == nullptr) return FinalPath;
	if (StartTriangle == DestTriangle) return {startPos, endPos};
	
	//=> Start looking for a path
	//Copy the graph
	const auto GraphCopy{pNavGraph->Clone()};
	
	//Create Extra node for the Start Node (Agent's position)
	//Create extra node for the endNode
	const auto StartIdx{GraphCopy->AddNode(std::make_unique<Node>(startPos))};
	const auto StartNode{GraphCopy->GetNodeAs<Node>(StartIdx)};
	const auto DestIdx{GraphCopy->AddNode(std::make_unique<Node>(endPos))};
	const auto DestNode{GraphCopy->GetNodeAs<Node>(DestIdx)};

	// Connect new nodes to the graph
	auto connectNodeToTriangleEdges = [&](size_t nodeIdx, const auto& triangle) {
		std::ranges::for_each(triangle->GetEdges(), [&](const auto& Edge) {
				auto OptionalIndex = pNavGraph->GetNavPolygon()->FindEdgeIndex(Edge);
				if (!OptionalIndex.has_value()) return;
				const auto EdgeIndex = OptionalIndex.value();
				const auto NodeId = pNavGraph->GetNodeIdFromEdgeIndex(EdgeIndex);

				if (NodeId > 0 && nodeIdx > 0)
				{
					GraphCopy->AddConnection(nodeIdx, NodeId);
				}
			}
		);
	};

	connectNodeToTriangleEdges(StartIdx, StartTriangle);
	connectNodeToTriangleEdges(DestIdx,  DestTriangle);
	GraphCopy->SetConnectionCostsToDistances();

	//Run A star on new graph
	const auto PathFinder = std::make_unique<AStar>(GraphCopy.get(), HeuristicFunctions::Chebyshev);
	const auto FoundPath = PathFinder->FindPath(StartNode, DestNode);

	FinalPath.reserve(FoundPath.size());
	std::ranges::transform(std::begin(FoundPath), std::end(FoundPath), std::back_inserter(FinalPath), [] (auto &Node)
	{
		return Node->GetPosition();
	});
	FinalPath.emplace_back(DestNode->GetPosition());
	
	return FinalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}