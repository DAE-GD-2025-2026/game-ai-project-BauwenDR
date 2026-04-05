#include "BFS.h"

#include <map>
#include <queue>
#include <unordered_set>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

std::vector<Node*> BFS::FindPath(Node const * const pStartNode, Node const * const pDestinationNode) const
{
	std::vector<Node*> path;
	if (!pStartNode || !pDestinationNode) return path;
	int startId = pStartNode->GetId();
	int destId = pDestinationNode->GetId();
	if (startId == destId) {
		path.push_back(pGraph->GetNode(startId).get());
		return path;
	}

	std::queue<int> q;
	std::unordered_map<int,int> parent;     // childId -> parentId
	std::unordered_set<int> visited;

	q.push(startId);
	visited.insert(startId);
	parent[startId] = -1; // sentinel

	bool found = false;
	while (!q.empty() && !found) {
		int current = q.front(); q.pop();

		// iterate all outgoing connections from current
		auto connections = pGraph->FindConnectionsFrom(current); // use original graph
		for (const auto &connPtr : connections) {
			int neighbor = connPtr->GetToId();
			if (visited.find(neighbor) != visited.end()) continue;
			visited.insert(neighbor);
			parent[neighbor] = current;
			if (neighbor == destId) {
				found = true;
				break;
			}
			q.push(neighbor);
		}
	}

	if (!found) return path; // empty: no path

	// Reconstruct path from dest to start
	for (int at = destId; at != -1; at = parent[at]) {
		path.push_back(pGraph->GetNode(at).get());
	}
	std::reverse(path.begin(), path.end());
	return path;
}

