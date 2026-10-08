

# BDD-Based Combinational Equivalence Checker (CEC)

A C++ project for verifying the functional equivalence of combinational logic circuits represented as **AND-Inverter Graphs (AIGs)**. The core engine utilizes a **Reduced Ordered Binary Decision Diagram (ROBDD)** package to perform canonical logic checks and handles multi-output circuit output permutations.

Developed as part of **CS-472: Design Technologies for Integrated Systems**.

---

## 🚀 Key Features & Implementation Highlights

* **Custom ROBDD Package (`bdd.hpp`):**
* **Canonical Representations:** Enforces fixed variable ordering x_0 < x_1 < .... < x_{n-1} and subgraph reduction using a `unique_table` lookup.


* **Recursive Boolean Operations:** Complete implementation of `NOT`, `AND`, and `XOR` logic operators leveraging If-Then-Else (`ITE`) primitives and recursive cofactor branching.




* **AIG-to-BDD Translation (`checker.hpp`):**
* Traverses multi-level AND-Inverter Graphs and builds canonical ROBDD representations for all primary output nodes.




* **Combinational Equivalence Checking (`checker.hpp`):**
* **Single & Multi-Output Verification:** Evaluates two AIGs to determine if their primary outputs produce identical logical functions for all input combinations.


* **Output Permutation Matching:** Automatically tests and resolves one-to-one output correspondence for multi-output networks.


* **Simulation Pre-Filtering (Performance Optimization):** Incorporates 64-bit signature simulation (`simulator.hpp`) to eliminate obvious non-equivalent candidate pairings prior to expensive BDD construction.



---

## 📁 Repository Structure

```text
.
├── bdd.hpp           # ROBDD data structures and logical operations (NOT, AND, XOR)
├── checker.hpp       # AIG-to-BDD construction & equivalence checking logic
├── aig.hpp           # AIG network data structure (aig_network_t, gate_t, signal_t)
├── simulator.hpp     # AIG 64-bit signature simulation engine
├── truth_table.hpp   # Truth table utilities (for N <= 6 PIs)
├── reader.hpp        # AIG file parsers
├── main.cpp          # Testbench driver and CLI binary entry point
├── CMakeLists.txt    # Modern CMake build configuration
└── Makefile          # Project Makefile

```

---

## 🛠️ Building & Running

### Requirements

* **Compiler:** C++11 compliant compiler (GCC / Clang)


* **Build System:** `make` or `cmake` ($\ge 3.10$)

### Compilation via Makefile

To compile the equivalence checker executable (`simulator`):

```bash
make

```

### Running Tests

Execute the built-in test suite across all benchmark circuits:

```bash
cd build
chmod +x ./simulator
./simulator ../benchmark/testXXX 

```

### Building via CMake

```bash
mkdir build && cd build
cmake ..
make -j
./simulator

```

---

## 📐 Architecture & Verification Flow

1. **AIG Parsing:** Loads circuit netlists as AND-Inverter Graphs.


2. **Signature Simulation:** Assigns exhaustive/random 64-bit patterns to Primary Inputs (PIs) to produce node signatures.
3. **Canonical BDD Generation:** Builds canonical ROBDD nodes gate-by-gate.


4. **Equivalence Comparison:** Compares root BDD indices across primary output nodes to guarantee exact functional alignment.
