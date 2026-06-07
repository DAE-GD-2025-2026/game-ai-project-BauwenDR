# Extra in the project
- 5 - Pathfinding: Fallback path

# Game AI Project
This repository contains my Game AI Programming (Algorithms 2) assignments.
Each assignment has one (or more levels).

## Levels
### Steering
A scene with dedicated controls to test each individual steering behaviour separately.
Includes UI toggles for behaviours such as Seek, Flee, Arrive, Pursuit, Evade abd Wander. Each behaviour can be enabled in isolation and configured with parameters, so you can observe how parameter changes affect motion.

### Combined Steering
Two arrow actors:
- One actor is controlled indirectly by setting its target with the mouse cursor and left-click.
- UI options allow selecting which of the two behaviours has priority.
- The second actor uses a wander behaviour but prioritizes avoiding collisions with the first actor.

### Flocking
A scene with a single wandering agent and a flock of agents. The flock prioritizes avoiding the rogue wandering agent; when not evading, the flock behaves like a standard bird flock.

### Graph Theory
Create a graph and have an actor follow it using a path-following steering behaviour (a seek agent moving between targets). The agent follows the path only if it is Eulerian.

### PathFinding
A tile grid containing normal, dirt, and water tiles. A* is used for navigation; movement is slower on dirt and water tiles are impassable.

### Navmesh
A scene where an actor uses the calculated navmash to navigate around the scene.

## Running
- Open the .uproject file in VS or Rider (or any other IDE, just never launch the uproject directly in Unreal Engine)
- In case the IDE is setup correctly, the build target should popup and it is as easy as just launching the project.
