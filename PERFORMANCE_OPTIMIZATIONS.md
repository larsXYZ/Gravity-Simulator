# Performance Optimizations

## Completed

### Static RNG (thread_local static generators)

**Status: DONE - ~40% speedup at 500 planets, ~28% at 1000**

In `planet.cpp`, `Effect.h`, and `space.cpp`, `std::random_device` and engine were recreated on every call, hitting the OS entropy pool repeatedly. Fixed by making them `thread_local static`.

### Newton's Third Law Optimization

**Status: TESTED - No improvement due to OpenMP overhead**

Computing each pair once (j > i) and applying symmetrically halves the gravity calculations, but requires thread-local accumulation buffers and a critical-section reduction. The overhead of allocating per-thread arrays and serializing the merge negated the gains. Splitting into separate gravity and collision passes also didn't help — the cost of iterating N^2 pairs twice outweighed the sqrt savings. Not worth pursuing without a fundamentally different parallelization strategy.

## Remaining Opportunities

### 1. O(n^2) Gravity - Barnes-Hut Algorithm (Biggest Win)

The nested loop in `space.cpp` computes all pairwise interactions. Replace with a **Barnes-Hut tree** (quadtree-based approximation), reducing complexity from O(n^2) to O(n log n). For distant clusters of bodies, approximate their combined gravity as a single node.

### 2. Spatial Indexing for Collisions & Particles

Particle-planet interactions (`particle_container.h`) check every particle against every planet. A **spatial hash grid** or quadtree would skip distant pairs, turning O(p * n) into closer to O(p).

### 3. Planet Lookup - Use a HashMap

`findPlanetPtr()` at `space.cpp` does O(n) linear search by ID. Replace with `std::unordered_map<int, size_t>` for O(1) lookups.

### 4. Vertex Array Caching for Particles

`particle_container.h` rebuilds the entire `sf::VertexArray` from scratch every frame. Maintain a persistent vertex array and only update moved particles.

### 5. MST Recalculation Caching

Prim's MST algorithm in `space.cpp` runs every frame when life rendering is enabled. Cache the result and only recalculate when the colony set changes.

## Benchmark Results

| Planets | Baseline | After Static RNG | Speedup |
|---------|----------|-------------------|---------|
| 100     | 0.26 ms  | 0.27 ms           | ~same   |
| 500     | 28.7 ms  | 17.1 ms           | 1.68x   |
| 1000    | 44.5 ms  | 32.2 ms           | 1.38x   |

Full reports with graphs are in `benchmark_reports/`.
