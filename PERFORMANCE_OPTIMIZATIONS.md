# Performance Optimizations

## 1. O(n^2) Gravity - Barnes-Hut Algorithm (Biggest Win)

The nested loop in `space.cpp:210-263` computes all pairwise interactions. For large planet counts, replace this with a **Barnes-Hut tree** (quadtree-based approximation), reducing complexity from O(n^2) to O(n log n). For distant clusters of bodies, you approximate their combined gravity as a single node. This is the standard approach for N-body simulators.

## 2. Random Number Generator Reuse

In `planet.cpp:474-478` and `space.cpp:1145-1158`, a new `std::random_device` and `std::mt19937` are created **every call**. `random_device` is especially expensive (may hit OS entropy pool). Fix: use a `thread_local static` generator:

```cpp
thread_local static std::mt19937 gen(std::random_device{}());
```

This is a quick, high-impact fix.

## 3. Spatial Indexing for Collisions & Particles

Particle-planet interactions (`particle_container.h:70-163`) check every particle against every planet. A **spatial hash grid** or quadtree would let you skip distant pairs entirely, turning O(p * n) into something much closer to O(p).

## 4. Planet Lookup - Use a HashMap

`findPlanetPtr()` at `space.cpp:818-830` does O(n) linear search by ID. Replace with an `std::unordered_map<int, size_t>` mapping ID to index for O(1) lookups.

## 5. Vertex Array Caching for Particles

`particle_container.h:165-208` rebuilds the entire `sf::VertexArray` from scratch every frame. Instead, maintain a persistent vertex array and only update vertices for particles that moved (the decimation system already tracks which bucket is active).

## 6. MST Recalculation Caching

The Prim's MST algorithm at `space.cpp:1195-1242` runs every frame when life rendering is enabled. Cache the result and only recalculate when the colony set changes.

## 7. Newton's Third Law Optimization

In the Phase 2 loop, you compute force of j on i, but also later compute force of i on j. Since F_ij = -F_ji, you can compute each pair only once (loop `j` from `i+1` to `n`) and apply the force symmetrically. This halves the work. Note: requires care with OpenMP to avoid race conditions on the acceleration array.

## Quick Wins Summary

| Change | Effort | Impact |
|--------|--------|--------|
| Static RNG | 5 min | Medium - removes OS calls per particle spawn |
| Newton's 3rd law (half pairs) | 30 min | ~2x speedup on gravity |
| HashMap for planet lookup | 15 min | Small-medium, depends on call frequency |
| Cache MST | 15 min | Small, only when life rendering on |
| Barnes-Hut tree | Hours | Massive for large N (100+ bodies) |
| Spatial hash for particles | Hours | Large when many particles + planets |
