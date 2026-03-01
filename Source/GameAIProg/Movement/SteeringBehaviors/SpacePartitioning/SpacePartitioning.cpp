#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, CellOrigin(-Width / 2, -Height / 2)
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, CellWidth{Width / Cols}
	, CellHeight{Height / Rows}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);

	Cells.reserve(Rows * Cols);
	for (int Row{0}; Row < Rows; ++Row)
	{
		for (int Col{0}; Col < Cols; ++Col)
		{
			Cells.emplace_back(CellOrigin.X + CellWidth * Col, CellOrigin.Y + CellHeight * Row, CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent* Agent)
{
	const auto CellIndex{PositionToIndex(Agent->GetPosition())};
	Cells[CellIndex].Agents.insert(Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent* Agent, const FVector2D& OldPos)
{
	const FVector2D CurrentPos = Agent->GetPosition();
	const int OldCellIndex = PositionToIndex(OldPos);
	const int NewCellIndex = PositionToIndex(CurrentPos);

	if (OldCellIndex != NewCellIndex)
	{
		Cells[OldCellIndex].Agents.erase(Agent);
		Cells[NewCellIndex].Agents.insert(Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent* Agent, float QueryRadius)
{
	NrOfNeighbors = 0;

	const auto Position{Agent->GetPosition()};

	const int StartRow{FMath::Clamp(static_cast<int>((Position.Y - CellOrigin.Y - QueryRadius) / CellHeight), 0, NrOfRows-1)};
	const int EndRow{FMath::Clamp(static_cast<int>((Position.Y - CellOrigin.Y + QueryRadius) / CellHeight), 0, NrOfRows-1)};
	const int StartCol{FMath::Clamp(static_cast<int>((Position.X - CellOrigin.X - QueryRadius) / CellWidth), 0, NrOfCols-1)};
	const int EndCol{FMath::Clamp(static_cast<int>((Position.X - CellOrigin.X + QueryRadius) / CellWidth), 0, NrOfCols-1)};
    
	for (int Col{StartCol}; Col <= EndCol; ++Col)
	{
		for (int Row{StartRow}; Row <= EndRow; ++Row)
		{
				for (ASteeringAgent* Neighbor : Cells[NrOfCols * Row + Col].Agents)
				{
					if (Neighbor != Agent)
					{
						const float Distance = FVector2D::DistSquared(Neighbor->GetPosition(), Position);
						if (Distance <= QueryRadius * QueryRadius)
						{
							Neighbors[NrOfNeighbors] = Neighbor;
							++NrOfNeighbors;
						}
					}
			}
		}
	}

}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	const FVector CellExtent{CellWidth / 2, CellHeight / 2, 0.0f};
	
	for (const auto &Cell : Cells)
	{
		const FVector CellOrigin{CellExtent + FVector{Cell.BoundingBox.Min.X, Cell.BoundingBox.Min.Y, 0.0}};

		DrawDebugBox(pWorld, CellOrigin, CellExtent, FColor::Red);
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	const int Row{FMath::Clamp(static_cast<int>((Pos.Y - CellOrigin.Y) / CellHeight), 0, NrOfRows-1)};
	const int Col{FMath::Clamp(static_cast<int>((Pos.X - CellOrigin.X) / CellWidth), 0, NrOfCols-1)};
	
	return NrOfCols * Row + Col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}