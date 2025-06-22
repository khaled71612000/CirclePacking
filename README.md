![Presentation website](https://github.com/user-attachments/assets/76e1e000-e03c-4ee7-b3a3-457f43e81844)

# Procedural Visual Systems in Unreal Engine

## Overview
This project implements several procedural generation techniques using Unreal Engine 5. Each system is visualized in real time through instanced meshes, glowing materials, and evolving geometry. The goal is to make algorithmic beauty both interactive and practical — from data visualization to gameplay tools.

![giphy](https://github.com/user-attachments/assets/cf298724-d8d8-4a43-80d4-29de55e19e7c)
---

## Systems & Classes

### 🌀 APrimeSpiralActor
Visualizes the **Ulam Spiral**, where prime numbers are plotted in a spiral pattern on a 2D grid.
![Youtube](https://youtu.be/HSi5PujFJeQ?si=QN9bC4YaNrDNks6_)

**Key Methods:**
- `Tick()`: Spawns and scales each prime-number cube in sequence.
- `IsPrime(int32 Number)`: Determines if a number is prime.
- `GetUlamSpiralPosition(int32 Index)`: Calculates the spiral position of an index.


**Use Case:**
- Prime-based puzzles, number visualizations, or visual encoding of index-based systems.

---

### 🌿 APoissonSpawner
Implements **Poisson Disc Sampling**, generating a natural, non-overlapping distribution of objects across a plane.
![Youtube](https://youtu.be/AqyTtPGKBjc?si=Eb0MLUgSW7lzCm9e)

**Key Methods:**
- `BeginPlay()`: Seeds the grid with starter points.
- `Tick()`: Iteratively spawns new points and instances.
- `GenerateNextPoints()`: Samples candidates around existing points.
- `IsInNeighborhood()`: Ensures spacing from neighbors.

**Use Case:**
- Forests, NPC camps, rock scatter, loot placement, or level layout zones with clean spacing.

---

### 🧬 ADLAClusterActor
Simulates **Diffusion-Limited Aggregation (DLA)** — particles (walkers) move randomly and "stick" to form coral/crystal-like structures.
![Youtube](https://youtu.be/Y16Zl75pgoI?si=GtO5hb0ysynVB6_0)

**Key Methods:**
- `SimulateStep()`: Runs a parallel simulation of walker movement.
- `IsAdjacentToAggregate()`: Checks if a walker touches the existing cluster.
- `AddInstanceToMesh()`: Adds a cube with scale growth animation.

**Use Case:**
- Procedural corruption, crystal growth, or visualizing biological processes.

---

### 🎈 ACirclePackingManager
Generates **non-overlapping growing circles** that fill space organically.
![Youtube](https://youtu.be/tIiBak3cNtk?si=BCBtOcw72ykh53GJ)

**Key Methods:**
- `TrySpawnNewCircle()`: Picks random positions and sizes using exponential bias.
- `IsOverlapping()`: Rejects circles that would collide with others.
- `Tick()`: Grows, ages, and shades all circles using custom mesh data.

**Use Case:**
- UI layouts, spawn zone visualizers, abstract territory systems.

---

## Features

- 🧠 **Procedural Algorithms** – Powered by math, visualized through Unreal Engine
- ⚙️ **Instanced Static Meshes** – Optimized performance for high counts
- 🔁 **Real-Time Simulation** – Every system runs live during gameplay
- 🌈 **Dynamic Visual Feedback** – Glow, scale, and debug visuals make logic visible
- 🛠️ **Fully Modular** – Drop any class into a scene to test or extend it

---

## References & Credits
This project is based on well-established mathematical and visual research:
- 🔵 **Circle Packing** – [Wikipedia](https://en.wikipedia.org/wiki/Circle_packing)  
- 🌿 **Poisson Disc Sampling** – [Jason Davies' demo](https://www.jasondavies.com/poisson-disc/)  
- 🧬 **Diffusion-Limited Aggregation (DLA)** – [Paul Bourke’s primer](https://paulbourke.net/fractals/dla/)  
- 🧮 **Ulam Spiral** – [JSTOR: Ulam's Original Paper](https://www.jstor.org/stable/2314055?origin=crossref)  
- 📚 **Mathematical Games Column (1964)** – [Scientific American](https://www.scientificamerican.com/article/mathematical-games-1964-03/)


Special thanks to the Unreal Engine community and procedural generation research.
![image](https://github.com/user-attachments/assets/742c3b89-8052-4f31-a4e7-16ad8af3f448)
![image](https://github.com/user-attachments/assets/dda2638c-df53-46f9-96e2-786e0f13ee4f)
![image](https://github.com/user-attachments/assets/55e89626-e16c-4e30-b30f-cc96ce821fad)
![image](https://github.com/user-attachments/assets/4a231de8-76ca-49a6-a8b0-d781c9a3b01b)
