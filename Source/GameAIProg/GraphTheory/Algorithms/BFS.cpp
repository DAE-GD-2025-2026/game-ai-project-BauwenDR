#include "BFS.h"

#include <map>
#include <queue>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

// TODO Breath First Search Algorithm searches for a path from the startNode to the destinationNode
std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	std::vector<Node*> Path{};
	Graph GraphCopy(pGraph->Clone());
	int CurrentNodeId = pGraph->GetNodes().front()->GetId();

	std::queue<int> NodeQueue;
	NodeQueue.push(CurrentNodeId);

	while (!NodeQueue.empty())
	{
		CurrentNodeId = NodeQueue.front();
		NodeQueue.pop();

		if (CurrentNodeId == pDestinationNode->GetId())
		{
			Path.push_back(pGraph->GetNode(CurrentNodeId).get());
			break;
		}

		auto Connections = GraphCopy.FindConnectionsFrom(CurrentNodeId);

		if (!Connections.empty()) {
			Path.push_back(GraphCopy.GetNode(CurrentNodeId).get()); // Get Node from original graph

			int NeighborId = Connections.front()->GetToId(); // Choose first available neighbor
        
			GraphCopy.RemoveConnection(CurrentNodeId, NeighborId);

			NodeQueue.push(NeighborId);
		}
	}

	std::ranges::reverse(Path);
	return Path;
}
