#include "NavGraph.h"

#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*static_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	// 0. Clear any existing graph data
    Nodes.clear();
    Connections.clear();

	// 1. Go over all the edges of the navigation mesh and create nodes
    std::vector<int> EdgeToNodeId{};
    EdgeToNodeId.resize(pNavPoly->GetEdges().size(), -1);

    for (size_t EdgeIndex{0}; EdgeIndex < pNavPoly->GetEdges().size(); ++EdgeIndex)
    {
        const auto &Edge{pNavPoly->GetEdges()[EdgeIndex]};
        const FVector P1{Edge.GetP1(*pNavPoly)};
        const FVector P2{Edge.GetP2(*pNavPoly)};

        const FVector2D Midpoint{ (P1.X + P2.X) * 0.5f, (P1.Y + P2.Y) * 0.5f };

        const int NewNodeId{AddNode(std::make_unique<NavGraphNode>(Midpoint, EdgeIndex))};
        EdgeToNodeId[EdgeIndex] = NewNodeId;
    }

	// 2. Create connections now that every node is created	
		// 2 valid nodes -> 1 connection
		// 3 valid nodes -> 3 connections
	for (const auto &Triangle : pNavPoly->GetTriangles())
    {
        auto TriEdges{Triangle.GetEdges()};

        // Collect valid node ids for this triangle's edges
        std::array<int, 3> NodeIds{};
        for (int NodeIndex{0}; NodeIndex < 3; ++NodeIndex)
        {
            auto OptEdgeIdx = pNavPoly->FindEdgeIndex(TriEdges[NodeIndex]);
            NodeIds[NodeIndex] = OptEdgeIdx ? EdgeToNodeId[*OptEdgeIdx] : -1;
        }

        std::vector<int> Valid{};
        for (int NodeIndex{0}; NodeIndex < 3; ++NodeIndex)
            if (NodeIds[NodeIndex] >= 0) Valid.push_back(NodeIds[NodeIndex]);

        std::ranges::sort(Valid);
        Valid.erase(std::ranges::unique(Valid).begin(), Valid.end());

        auto connectPair = [&](int const Lhs, int const Rhs)
        {
            if (Lhs == Rhs) return;
            if (!FindConnection(Lhs, Rhs))
            {
                AddConnection(Lhs, Rhs);
            }
        };

        if (Valid.size() == 2)
        {
            connectPair(Valid[0], Valid[1]);
        }
        else if (Valid.size() == 3)
        {
            connectPair(Valid[0], Valid[1]);
            connectPair(Valid[1], Valid[2]);
            connectPair(Valid[2], Valid[0]);
        }
    }

	// 3. Set the connections cost to the actual distance
	SetConnectionCostsToDistances();
}
