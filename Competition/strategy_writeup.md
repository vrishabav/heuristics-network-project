<img width="617" height="283" alt="image" src="https://github.com/user-attachments/assets/bd84180d-e776-44d6-8705-0ac4af624727" />
# Heuristics Guild Application: Contest Writeup

## 1. Understanding the Problem

This is essentially a two-phase NP-hard optimization challenge combining a 2D bin-packing variant with a min-max assignment problem.

Components Involved:
* **Grid:** A facility with $R$ parallel racks (rows), each having $S$ slots. Some slots are blocked ("Dead Slots").
* **Servers:** $M$ blocks, each with a physical length ($z_i$) and a computing capacity ($c_i$). A server must fit entirely within one rack, occupying consecutive, non-dead slots.
* **Pools:** The servers must be divided among $P$ pools. 
* **Objective:** Any single rack can fail at any time, taking all its assigned servers offline. For any given pool $i$, its **guaranteed power** ($gp_i$) is its total power minus the maximum power it would lose if a *single* rack went offline. 

Mathematically, the guaranteed power for pool $i$ is:
$$gp_i = \sum_{k \in i} c_k - \max_{0 \le r < R} \left( \sum_{k \in i \text{ and in rack } r} c_k \right)$$

Goal is to maximize the bottleneck i.e. maximize the minimum $gp_i$ across all pools. Because there is no perfect greedy strategy for this type of complex combinatorial optimization, I expected a meta-heuristic like Simulated Annealing (SA) would be the best approach.

---

## 2. Initial Strategy (which ended up with TLE)
**Partial result: 5 points**

My first approach focused on simply establishing a valid state and wrapping it in a Simulated Annealing loop. However, it resulted in a TLE on test case 2.

### Placement Bottleneck
To place the servers initially, I wrote a greedy packing algorithm using bitsets. To place a server of length $z$, I shifted a bitmask across every possible position in every rack, checking for overlaps with dead slots or other servers.

Here, the worst-case time complexity was $O(M \times R \times S^2 / 64)$. For $M=10^6$, $R=1000$, and $S=1000$, this resulted in over a trillion operations. It was systematically checking racks for servers that had zero chance of fitting.

### Simulated Annealing Bottleneck
For the optimization phase, my SA evaluation function calculated the rack losses from scratch every single time I tested a move, iterating over all $R$ racks to find the maximum loss.

The complexity was $O(R)$ per move. With $R=1000$ and an SA loop that needed to run millions of times to find a good state within the 10-second limit, this added up to $\sim 10^{10}$ operations, completely destroying my time budget.

---

## 3. Best Strategy & Step-by-Step Logic
**Partial result: 239,768 points**

To fix the TLE, I fundamentally changed how the algorithm managed states. My goal was to drop the time complexity of both the bin-packing and the SA evaluation so I could minimize the time taken in each iteration, maximizing the number of states explored. Here is exactly how I built it.

### Step 1: The Grid Setup & Amortized $O(M \log S)$ Placement
Instead of checking every slot with bitsets, I had each rack maintain two data structures:
1. A `set<pair<int,int>> seg` tracking intervals of free space (e.g., "Slots 0 to 10 are free").
2. A `multiset<int> slen` tracking the lengths of those free spaces.

When processing dead slots, I used custom `seg_del` and `seg_add` lambdas to split free intervals. If a dead slot hit index 5, `[0, 10]` became `[0, 4]` and `[6, 10]`. 
When placing servers, I sorted them by capacity descending (`c >`), prioritizing the most powerful servers. For each server, I could check the largest free space in a rack's `slen` multiset in $O(1)$ time. If it didn't fit, I skipped the rack instantly. If it did, I found the leftmost gap and updated the intervals in $O(\log S)$ time.

### Step 2: Balanced Initial Pool Assignment
Instead of blindly assigning pools globally, I needed to give the Simulated Annealing engine a stable starting point. 
I grouped the placed servers by rack, sorted them by capacity (descending), and assigned pools in a round-robin fashion ($0, 1, \dots, P-1$) within each rack. This guaranteed that no single rack dominated any pool from the very beginning, providing a highly balanced starting state without over-optimizing it.

### Step 3: Fast State Tracking (The Secret Sauce)
The biggest leap came from tracking the SA state using global `multiset`s. To avoid recounting the whole data center on every move, I tracked:
* `pool_total[p]`: Total power of pool `p`.
* `pool_rack[p][r]`: Power of pool `p` sitting in rack `r`.
* `rack_ms[p]`: A multiset of the rack totals for pool `p`. The maximum element (`*rack_ms[p].rbegin()`) instantly gave me the worst-case rack loss!
* `gp_ms`: A global multiset of all $P$ pools' guaranteed power. The minimum element (`*gp_ms.begin()`) was my objective score.

I wrote a `move_to` function. When moving a server, it only subtracted its capacity from its old pool's trackers and added it to its new pool's trackers. What used to cost $O(R)$ now cost $O(\log R + \log P)$.

### Step 4: Simulated Annealing (The Core Engine)
With evaluations practically instantaneous, I built the SA loop to run for the duration of the time limit (`while(t_now < 9.2)`).
* **The Cooling Schedule:** I used `temp = 500.0 * pow(0.001, t_now / 9.2)`. It starts hot (willing to accept bad moves to explore) and cools down to only accept strict improvements.
* **Expanding the Neighborhood:** I defined two main move types. About 66% of the time, I moved a single server to a new pool (great for broad exploration). The other 33% of the time, I swapped the pools of two servers. Swaps keep the overall pool capacities identical while shuffling the internal rack risks, preventing the algorithm from oscillating between invalid states.
* **The Acceptance Logic:** `if (delta < 0 && ud(rng) >= exp((double)delta / temp))` i.e. if a move worsened the score, I calculated a probability against the current temperature. If rejected, I reverted the state via `move_to`. If accepted, the algorithm successfully jumped out of a local optimum trap.

---

## 4. Attempted Improvements (and why they likely regressed)
**Partial result: 156,495 points**

I attempted to introduce several logical "optimizations" but these resulted in a score drop. Here is a breakdown of what I tried and theories as to why it failed:

### Attempt 1: Length Descending Placement
* **What I tried:** I sorted servers primarily by length (descending), tie-broken by capacity, placing the hardest-to-fit servers first.
* **Why it failed:** While placing large blocks first packs the physical grid tighter, it inherently groups massive-capacity servers together. This drastically reduced the flexibility my SA had to balance the pools later. SA thrives on having many small, modular pieces it can swap around; length-first placement forced it to work with clunky, rigid blocks.

### Attempt 2: Min-Heap Initial Pool Assignment
* **What I tried:** During initial assignment, I assigned each server to the pool with the *least* current rack capacity using a priority queue, hoping for perfect initial balance.
* **Why it failed:** This over-optimized the initial state. Simulated Annealing needs a bit of noise to start. By perfectly flattening the initial rack distributions, my SA started in a deep local optimum. Almost any random move it tried immediately lowered the score, causing the algorithm to reject too many moves early in the temperature schedule.

### Attempt 3: Targeted SA Moves
* **What I tried:** I altered the SA logic so that 25% of the time, it forcefully picked a server from the "worst" pool and moved it, rather than picking completely at random.
* **Why it failed:** Simulated Annealing derives its power from random, unbiased walks through the state space. By forcing the algorithm to aggressively target the worst pool, the inherent randomness/Markov Chain nature of the SA got lost as it tended into a too greedy algorithm.

### Attempt 4: Cyclic Swaps, Adaptive Reheating, and State Tracking
**Partial result: 235,510 points**
* **What I tried:** I observed that binary swaps can only reach states that differ by a transposition, meaning there exist swap-stable local optima that binary swaps cannot escape. I introduced a 3-way cyclic swap. I also added adaptive spike reheating i.e. applying a 20% multiplier to the temperature for a 1000-iteration window whenever the acceptance rate collapsed below 2%. Finally, I implemented strict best-solution tracking so I could restore the absolute best state at the end.
* **Why it failed:** The extra overhead of evaluating a 3-way swap delta may have reduced my total iteration count. Also adaptive reheating, while preventing premature freezing, sometimes kicked the algorithm out of a highly productive narrow basin right when it needed to polish a good solution, resulting in slightly lower scores than a pure, fast cooling schedule (similar to issues with SGD as opposed to MBGD).

### Attempt 5: Warm-up Pass and Multi-Restart SA
**Partial result: 234,236 points**
* **What I tried:** I added a shallow greedy "warm-up" pass to fix large imbalances before the SA even started. Then, I implemented a two-phase SA (multi-restart). After the initial cooling phase, I perturbed roughly 10% of the server assignments using $n/10$ binary swaps to escape the local basin, and restarted the cooling curve at a lower initial temperature ($T=20$).
* **Why it failed:** Similar to the min-heap attempt, the warm-up pass stripped away the organic noise that SA relies upon. The multi-restart perturbation proved too destructive; while 10% was meant to be a controlled scramble, it often destroyed the delicate, hard-won rack-balancing structures the first phase had built, and the remaining time budget was insufficient to reconstruct them.

---

## 5. Potential Future Directions

1. **Initial Temperature Calibration:** Currently, the initial temperature $T=500$ is a hardcoded default. A potential better approach would be to sample 200–500 random moves without applying them, compute the average absolute delta, and dynamically set $T_0 = \text{average\\_delta} / \ln(0.8)$ to target an 80% initial acceptance rate. This would eliminate input sensitivity, as $T=500$ might be a random walk for uniformly small capacities or too cold for massive capacities.
2. **Rack-Aligned Placement Cursor:** Instead of advancing the placement cursor round-robin for every single server, I could reset it to a random rack after placing $K$ consecutive servers. This creates a less predictable rack distribution, giving the SA a slightly different topological starting point on each run (especially useful if combined with multi-restarts).
3. **Weighted Objective Relaxation:** The bottleneck function ($\min(gp_i)$) hides all information at a single pool, meaning improvements to non-bottleneck pools score a delta of 0. For roughly 5% of iterations, I could evaluate the delta against a smooth surrogate like $\sum(gp_i) / P$. This allows the algorithm to build up a reserve of capacity "for free," which the bottleneck pool can later draw from.
4. **Lazy Rack-Domination Detection:** When a server moves, I currently update both affected pools. However, if the rack the server sits in is *not* the worst rack for the donor pool, that pool's score mathematically cannot decrease. Detecting this in $O(1)$ allows me to prune single-move proposals that cannot possibly hurt the donor pool, skipping the Boltzmann calculation entirely.

---

## 6. Conclusion
My highest score was achieved by prioritizing **iteration speed** and **unbiased randomness**. By using intervals for fast bin-packing and logarithmic data structures for state evaluation, the algorithm was free to evaluate millions of valid configurations organically. The later attempts showed the trap of trying to make the random moves "smarter" or over-optimizing the starting state, which often restricts the search space, traps the algorithm in local optima, and ultimately yields worse results. Allowing the SA to naturally cool and relying on blazing-fast evaluations proved to be the winning formula.
