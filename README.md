
# Project Shimmer

Shimmer is a scalable TCP MMO server framework for Unreal Engine–based games.
It’s built around an ECS architecture to handle networking, spatial simulation, and gameplay logic efficiently.
The goal is to provide a modern, high-performance backend capable of supporting large, persistent worlds.

Planned Features:
[b]Core Architecture[/b]

- Entity–Component–System design
- C++26 with reflection and modern async features
- Multithreaded job and task scheduling

[b]Networking[/b]

- Custom TCP transport with efficient serialization
- Connection and session management
- Replication and state synchronization

[b]World and Simulation[/b]
- Spatial partitioning (octree, bvh accelerators)
- Zones, instances
- Basic AI and pathfinding hooks

[b]Scripting and Services[/b]

- Coroutine-based scripting API
- Integration for microservices (chat, mail, etc.)
- Persistent data storage (SQL based, most probably postgres) and world snapshots

This list is subject to change as the project evolves.
