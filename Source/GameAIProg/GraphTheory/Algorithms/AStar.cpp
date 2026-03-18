#include "AStar.h"

#include <set>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::unordered_map<Node*, float> OpenList{};
	std::unordered_map<Node*, float> CloseList{};
	std::unordered_map<Node*, Node*> Path{};

	std::pair CurrentNode{pStartNode, 0};
	std::pair LowestHPoint{pStartNode, GetHeuristicCost(pStartNode, pGoalNode)};
	OpenList.insert(CurrentNode);

	while (!OpenList.empty())
	{
		CurrentNode = *std::ranges::min_element(OpenList, [](const auto &lhs, const auto &rhs) {return lhs.second < rhs.second;});

		if (CurrentNode.first == pGoalNode)
		{
			LowestHPoint = {CurrentNode.first, 0};
			break;
		}

		// Calculating the path
		const auto Connections{pGraph->FindConnectionsFrom(CurrentNode.first->GetId())};
		for (const auto &Connection : Connections)
		{
			const auto NextNode{pGraph->GetNode(Connection->GetToId()).get()};
			const float GCost{CurrentNode.second + Connection->GetWeight()};
			const float HCost{GetHeuristicCost(NextNode, pGoalNode)};
			const float FCost{GCost + HCost};

			if (CloseList.contains(NextNode) && GCost > CloseList[NextNode])
			{
				continue;
			}
			
			if (OpenList.contains(NextNode) && FCost > OpenList[NextNode])
			{
				continue;
			}

			if (CloseList.contains(NextNode))
			{
				CloseList.erase(NextNode);
			}

			if (OpenList.contains(NextNode))
			{
				OpenList.erase(NextNode);
			}

			if (HCost < LowestHPoint.second)
			{
				LowestHPoint = {NextNode, HCost};
			}

			OpenList.insert({NextNode, FCost});
			Path[NextNode] = CurrentNode.first;
		}
		
		OpenList.erase(CurrentNode.first);
		CloseList.insert(CurrentNode);
	}

	// Reconstructing the path
	std::vector<Node*> ReconstructedPath{};

	Node* CurrentPathNode{LowestHPoint.first};
	while (Path.contains(CurrentPathNode))
	{
		ReconstructedPath.push_back(CurrentPathNode);
		CurrentPathNode = Path[CurrentPathNode];
	}
	ReconstructedPath.push_back(pStartNode);

	std::ranges::reverse(ReconstructedPath);
	
	return ReconstructedPath;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}