--[[
 Programming Languages Labwork Assignment.
 Group 15 Members: 
 1. C026-01-0907/2025 Igamba Janice Muthoni.
 2. C026-01-0906/2025 Joy Wanjiru
 3.C026-01-0907/2025 Timothy Mbugua.


 Question 15: COOPERATIVE UNIVERSITY COMPUTING SHEDULER
  ---------------------------------------------------------------------

  Instructions followed in thei exersise as per the earlier given instructions:
    1.The coroutine.create / resume / yield / status are used meaningfully
     throughout the code.
    2.Two different scheduling policies (round-robin vs weighted priority) have been implemented and
      compared, each with its own reasoning as explained below.
    3.State management: each job's progress lives entirely inside its
      own coroutine's paused state.
    4.Error handling: Timetable-Generation job is deliberately made to fail mid-way to
      prove the return value of coroutine.resume() is actually checked
      and handled, not just decorative.
    5.Dead coroutines are never resumed: every resume call is guarded
      by an explicit coroutine.status() check beforehand.
    
]]


--PART (a): Represent every job as a coroutine.

--[[
Each job  performs ONE unit of work, then calls coroutine.yield() to hand control back to the scheduler. 
A job can be told to fail at a given step (failAtStep) to simulate a real-world fault (for example a corrupted input)
An Example is seen where there is a deliberate error in the Timetable-generation job.
 ]]

local function makeJob(name, steps, failAtStep)
    return coroutine.create(function()
        for step = 1, steps do
            if failAtStep and step == failAtStep then
                -- This simulates a genuine runtime fault inside a job, for example a corrupted record encountered mid-processing.
                error(string.format("%s hit a fatal fault at step %d (corrupted data)", name, step))
            end
            print(string.format("[%s] doing step %d/%d", name, step, steps))
            coroutine.yield()      
             --The  coroutine.yield() function  signals every operation to stop after completing a unit of work.
        end
        print(string.format("[%s] finished all work.", name))
        --The function returns here as the coroutine status becomes "dead"
    end)
end

--[[ 
Below are the four jobs from the scenario.
Here, Timetable-Generation is deliberately given a fault at step 3 ( "corrupted room-allocation record")
which is fictional in order to purely  demonstrate the scheduler's error-handling path.
]]

local jobDefs = {
    { name = "AI-Model-Simulation",     steps = 5, priority = 3 },
    { name = "Student-Results",         steps = 3, priority = 2 },
    { name = "Timetable-Generation",    steps = 4, priority = 1, failAtStep = 3 },
    { name = "Library-Indexing",        steps = 2, priority = 1 },
}


--[[ 
The safeResume function below is a shared helper that is used to help safely resume a single job's coroutine.
It ensures that: 
 1) We NEVER resume a coroutine that is already "dead".
 2) We ALWAYS check the return value of coroutine.resume() and handle
 a failed (errored) run in a good manner instead of crashing the
whole scheduler.
The function returns true if the job is still alive after this call and false if it should now be removed (finished normally OR errored out).
]]

local function safeResume(job)
    if coroutine.status(job.co) == "dead" then
        print(string.format("[%s] skipped ;This coroutine is already dead", job.name))
        return false
    end

    local ok, err = coroutine.resume(job.co)

    --[[
        if the job encounters an error, coroutine.resume returns false plus the error message when the
        job's function raised an error().
        The error is then handled as shown below instead of allowing it to propagate and crash the scheduler.
        ]] 
    if not ok then
        print(string.format("[%s] ERROR: %s", job.name, err))
        return false
    end

    return coroutine.status(job.co) ~= "dead"
end


-- PART (b): Creating/ Using a plain round-robin scheduler.

--[[
The logic behind round robin mimics the FIFO queue. It  involves 
poping the job at the front, giving it exactly one unit of work, and if it is
still alive, pushing it to the back of the queue so that every job re-enters the queue in
the same relative order. 
This guarantees strict turn-equality as no job can ever get two turns before another job still waiting gets one.
]]
local function runRoundRobin(defs)
    print("\n=== ROUND-ROBIN SCHEDULER ===")
    local queue = {}
    for _, def in ipairs(defs) do
        table.insert(queue, { name = def.name, co = makeJob(def.name, def.steps, def.failAtStep) })
    end

    local pass = 1
    while #queue > 0 do
        print(string.format("-- pass %d (active jobs: %d) --", pass, #queue))
        local passSize = #queue   -- one full pass = one turn per currently active job

        for _ = 1, passSize do
            local job = table.remove(queue, 1)   -- This removes a job from the front of the queue and then allows it to resume if it is still alive.

            local stillAlive = safeResume(job)

-- PART (d): This ensures that it only requeues if the coroutine is still alive. A dead/errored coroutine is dropped here permanently.
            if stillAlive then
                table.insert(queue, job)   -- This sends the jobs that are still alive  to back of the queue so that they can be executed again in the second round.
            else
                print(string.format("--> %s removed from scheduler (coroutine dead)", job.name))
            end
        end
        pass = pass + 1
    end
    print("=== All jobs complete or removed (round-robin) ===\n")
end


-- PART (c): Priority scheduler - This involves giving extra turns to the jobs with higher priority without causing starvation to the other low priority jobs.

--[[
Normally, a priority schedule policy that always runs the highest priority job first
 would starve low-priority jobs indefinitely if high-priority jobs
keep arriving.
In order to avoid that, this uses a WEIGHTED round-robin. This means that:
- Every job's priority becomes its "credit" count for the round.
-A job is resumed / is given bonus turns in a row, that are equal to its  `credit` times,
but EVERY job (regardless of its priority), is entered into that loop at least once per round,
guaranteeing forward progress and preventing priority starvation.
This is the standard fix for priority starvation.
]]
local function runPriorityScheduler(defs)
    print("\n=== PRIORITY SCHEDULER (extra turns, no starvation) ===")
    local jobs = {}
    for _, def in ipairs(defs) do
        table.insert(jobs, {
            name = def.name,
            co = makeJob(def.name, def.steps, def.failAtStep),
            priority = def.priority,
        })
    end

    local round = 1
    while #jobs > 0 do
        print(string.format("-- round %d (active jobs: %d) --", round, #jobs))
        local i = 1
        while i <= #jobs do
            local job = jobs[i]
            local credits = job.priority   -- bonus turns this round

            local stillAlive = true
            for turn = 1, credits do
                stillAlive = safeResume(job)
                if not stillAlive then break end
            end

-- PART (d): remove dead/errored coroutines automatically
            if not stillAlive then
                print(string.format("--> %s removed from scheduler (coroutine dead)", job.name))
                table.remove(jobs, i)  -- Here we donot increament the value of i as its initial value has now been occupied by a new value. Hence increamenting it would mean that the new value is not read.
            else
                i = i + 1
            end
        end
        round = round + 1
    end
    print("=== All jobs complete or removed (priority scheduler) ===\n")
end


-- PART (e): Run both and compare

runRoundRobin(jobDefs)
runPriorityScheduler(jobDefs)

print([[
COMPARISON between  Round-robin and Priority scheduling.

1.Round-robin:
  - Every active job gets exactly ONE step per pass, regardless of
    importance. AI-Model-Simulation (priority 3) makes no faster
    progress than Library-Indexing (priority 1) .Both wait an equal
    number of turns between steps.
  - Strictly fair and simple to reason about, and starvation-free by
    construction, but it cannot express urgency at all.

2.Priority scheduler:
  - High-priority jobs receive multiple consecutive turns ("credits")
    per round, so AI-Model-Simulation completes in fewer rounds overall.
  - Low-priority jobs are still guaranteed at least one turn every
    round. They are however slowed down relative to high-priority jobs, but
    never starved indefinitely.
  - It is slightly more complex as it needs a credit/weight system per round,
    and finishing order is less predictable. However, overall
    throughput better reflects real job importance.

3.Error handling in both schedulers:
  - Timetable-Generation was deliberately made to fail at step 3.
  - Because every resume goes through safeResume(), the failure is
    caught via the (ok, err) return values of coroutine.resume(),
    reported clearly, and the job is cleanly removed from scheduling.
    It does not crash the rest of the simulation or get resumed again
    after entering the dead state.

In Summary:
round-robin optimises for equality of turns while the priority
scheduler optimises for responsiveness of important jobs while still
guaranteeing forward progress for everyone else, and both schedulers
fail safely rather than crashing when a job errors out.
]])