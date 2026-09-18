# Programming Languages Labwork Assignment

**Question 15: Cooperative University Computing Scheduler**

## Group 15 Members

| No. | Registration No.        | Name                     |
|-----|--------------------------|---------------------------|
| 1   | C026-01-0907/2025        | Igamba Janice Muthoni     |
| 2   | C026-01-0906/2025        | Joy Wanjiru                |
| 3   | C026-01-0907/2025        | Timothy Mbugua              |

---

## Question 15 

## Overview of the Solution

The solution is written in **Lua** and uses the built-in coroutine library (`coroutine.create`, `coroutine.resume`, `coroutine.yield`, `coroutine.status`) to model each job as an independent, pausable unit of work.

Four jobs are simulated:

| Job Name              | Steps | Priority | Notes                                   |
|------------------------|:-----:|:--------:|-------------------------------------------|
| AI-Model-Simulation     | 5     | 3        | Highest priority                          |
| Student-Results         | 3     | 2        | Medium priority                           |
| Timetable-Generation    | 4     | 1        | Deliberately fails at step 3 (simulated fault) |
| Library-Indexing        | 2     | 1        | Lowest priority, shortest job              |

### Part (a) Jobs as Coroutines
---

Each job is created with `makeJob(name, steps, failAtStep)`, which wraps a loop inside `coroutine.create`. On every iteration the job:
1. Optionally raises a deliberate `error()` if the current step matches `failAtStep` (used to simulate a corrupted record in **Timetable-Generation**).
2. Prints that it has completed one unit of work.
3. Calls `coroutine.yield()` to hand control back to the scheduler.

Once the loop finishes, the coroutine's function returns and its status naturally becomes `"dead"`.

### Safe Resume Helper

A shared `safeResume(job)` function is used by **both** schedulers to guarantee:
- A coroutine already in the `"dead"` state is **never** resumed again.
- The return value of `coroutine.resume()` is **always checked**; if a job errors out (`ok == false`), the error message is reported and the job is safely dropped instead of crashing the whole scheduler.

### Part (b) Round-Robin Scheduler
---

`runRoundRobin(defs)` implements a classic FIFO queue:
- Each job is popped from the front of the queue and given exactly **one** unit of work.
- If still alive, it is pushed to the back of the queue, preserving turn order.
- This guarantees strict fairness meaning that no job can get two turns before every other active job has had one.

### Part (c) Priority Scheduler (Weighted, Starvation-Free)

`runPriorityScheduler(defs)` uses a **weighted round-robin** approach instead of strict priority-first scheduling (which would starve low-priority jobs):
- Each job's `priority` value becomes its number of **credits** (bonus consecutive turns) per round.
- Every job (regardless of priority ) is still visited once per round, guaranteeing forward progress for all jobs and preventing starvation.

### Part (d) Automatic Removal of Dead Coroutines

In both schedulers, once `safeResume()` reports that a job is no longer alive (finished normally **or** errored out), it is permanently removed from the active job list/queue and is never resumed again.

### Part (e) Comparison of Scheduling Policies

| Aspect | Round-Robin | Priority (Weighted) Scheduler |
|--------|-------------|-------------------------------|
| Fairness | Every job gets exactly one turn per pass, regardless of importance | High-priority jobs get more turns per round |
| Speed for important jobs | No faster progress for high-priority jobs | High-priority jobs (e.g. AI-Model-Simulation) finish in fewer rounds |
| Starvation risk | None - starvation-free by construction | None - every job gets at least one turn per round |
| Complexity | Simple, easy to reason about | Slightly more complex (credit/weight system) |
| Predictability | Highly predictable finishing order | Less predictable, but reflects real-world job importance |

**Error handling in both schedulers:** `Timetable-Generation` is deliberately made to fail at step 3. Because every resume call goes through `safeResume()`, the failure is caught via the `(ok, err)` values returned by `coroutine.resume()`, reported clearly, and the job is cleanly removed — without crashing the rest of the simulation or being resumed again after entering the dead state.

**Summary:** Round-robin optimises for equality of turns, while the priority scheduler optimises for responsiveness of important jobs while still guaranteeing forward progress for everyone else. Both schedulers fail safely rather than crashing when a job errors out.

---


Running the script will:
1. Execute the round-robin scheduler over all four jobs, printing each step as it happens.
2. Execute the priority (weighted round-robin) scheduler over the same four jobs.
3. Print a final comparison summary between the two scheduling policies.

---

## Key Lua Concepts Demonstrated
- `coroutine.create()` - wrapping each job's logic in an independent coroutine.
- `coroutine.resume()` - advancing a job by one unit of work; return values checked every time.
- `coroutine.yield()` - pausing a job after each unit of work, preserving its internal state.
- `coroutine.status()` - guarding against resuming coroutines that are already `"dead"`.
- Structured error handling with `error()` and `pcall`-style checking via `coroutine.resume()`'s boolean return value.