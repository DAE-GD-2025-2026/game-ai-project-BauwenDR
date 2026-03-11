#pragma once
#include <stack>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(const Eulerianity& GraphEulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		if (!IsConnected())
		{
			return Eulerianity::notEulerian;
		}

		int OddNodes{0};

		const std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		for (int Index{0}; Index < Nodes.size(); ++Index)
		{
			if ((m_pGraph->FindConnectionsFrom(Index).size() & 1) != 0)	// If size is uneven (when lsb is one)
			{
				OddNodes++;

				if (OddNodes > 2)
				{
					return Eulerianity::notEulerian;
				}
			}
		}

		if (OddNodes == 2 && Nodes.size() != 2)
		{
			if (Nodes.size() == 2)
			{
				return Eulerianity::eulerian;
			}
			
			return Eulerianity::semiEulerian;
		}

		return Eulerianity::eulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(const Eulerianity& GraphEulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		Graph GraphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = GraphCopy.GetActiveNodes();
		int CurrentNodeId{Graphs::InvalidNodeId};

		if (GraphEulerianity == Eulerianity::notEulerian)
		{
			return Path;
		}

		// Start at the first note with uneven nodes, if none are found, we take the last one
		CurrentNodeId = std::ranges::find_if(Nodes, [&GraphCopy](const auto &Node)
		{
			return (GraphCopy.FindConnectionsFrom(Node->GetId()).size() & 1) == 0;
		}) - Nodes.begin();
		
		std::stack<int> NodeStack;
		NodeStack.push(CurrentNodeId);

		while (!NodeStack.empty())
		{
			CurrentNodeId = NodeStack.top();

			auto Connections = GraphCopy.FindConnectionsFrom(CurrentNodeId);

			if (!Connections.empty()) {
				Path.push_back(m_pGraph->GetNode(CurrentNodeId).get()); // Get Node from original graph

				int NeighborId = Connections.front()->GetFromId(); // Choose first available neighbor
        
				GraphCopy.RemoveConnection(CurrentNodeId, NeighborId);

				NodeStack.push(NeighborId);
			} else {
				NodeStack.pop();
			}
		}

		if (!Path.empty()) {
			Path.push_back(m_pGraph->GetNode(CurrentNodeId).get()); // Get Node from original graph
		}

		std::ranges::reverse(Path);
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& Visited,
	                                           int StartIndex) const
	{
		Visited[StartIndex] = true;

		auto Connections{m_pGraph->FindConnectionsFrom(StartIndex)};
		for (const auto Connection : Connections)
		{
			const int NodeId { std::ranges::find_if(Nodes, [&Connection](const auto &Node) {return Node->GetId() == Connection->GetToId();}) - Nodes.begin()};

			if (NodeId >= Nodes.size()) continue;
			if (Visited[NodeId]) continue;

			Visited[NodeId] = true;
			VisitAllNodesDFS(Nodes, Visited, NodeId);
		}
		// TODO Tip: use an index-based for-loop to find the correct index
	}

	inline bool EulerianPath::IsConnected() const
	{
		const std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
		{
			return false;
		}

		std::vector<bool> VisitedNodes{};
		VisitedNodes.reserve(Nodes.size());

		VisitAllNodesDFS(Nodes, VisitedNodes, 0);

		return std::ranges::find_if(VisitedNodes, [](const bool Visited) { return !Visited; }) == VisitedNodes.end();
	}
}
