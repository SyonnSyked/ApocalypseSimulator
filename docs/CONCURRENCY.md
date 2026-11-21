# Concurrency foundation

The application creates and updates the world on the calling thread. No
parallel registry access is enabled. `PhaseExecutor` is a
reusable worker pool for future simulation phases, not an implicit guarantee
that code run through it is safe. Construct it once for the lifetime of the
future simulation; do not construct a pool per tick.

## Executor contract

`run(itemCount, batchSize, callback)` divides `[0, itemCount)` into stable,
non-overlapping half-open ranges. The last range may be shorter. The callback
receives the range and its zero-based batch index. A fixed item count and batch
size produce the same ranges regardless of worker count; execution order and
worker assignment are unspecified. An empty range dispatches no work. Zero
workers, zero batch size, and an empty callback are rejected.

A call waits until every batch finishes, even when a callback throws. The first
exception in batch-index order is rethrown to the caller after the barrier.
There is no rollback: discard partial outputs or otherwise handle them before
continuing. The pool remains usable after an exception. Calls from multiple
external threads are serialized. Nested dispatch from a worker callback is
rejected, including dispatch to another executor. A callback must be safe to
invoke concurrently, including any state captured by reference or stored in a
mutable callable. Callers must keep the executor, callback dependencies, and
input/output storage alive until `run` returns; destruction must not race a
call to `run` or occur inside a callback.

## Rules for future simulation phases

1. The simulation owner prepares a stable list of work and immutable inputs
   before dispatch. Workers may read shared input and write disjoint output
   slots, or write exclusively to buffers indexed by batch. Do not resize shared
   containers or touch the same output object concurrently.
2. Wait for `run` to return before reading outputs or beginning a dependent
   phase. Merge batch buffers in batch-index order if ordering matters.
3. Resource requests, movement, interactions, and other contested changes must
   be resolved by an explicit owner or a later merge/commit phase. A person
   decision must not directly decrement a shared district's food or population.
4. Do not create or destroy EnTT entities, add or remove components, create
   pools/views that mutate registry state, or change storage layout while
   workers access the registry. Build work lists and prepare storage before a
   phase; apply structural changes at a barrier. Disjoint component writes and
   concurrent reads still require an audit of EnTT's guarantees and all touched
   data, not just a call to the executor.
5. Keep initialization on one thread until its registry mutations and shared
   district counts have an explicit partition and commit design. In particular,
   `CreatePopulation` currently increments district population directly.
6. For repeatable simulations, introduce a configured seed and stable entity or
   event IDs, and derive random draws from those IDs and the tick. Worker-local
   random generators alone do not make outcomes independent of scheduling.
   Stable resolution order and floating-point behavior also matter.

Use a single-worker execution as a reference and compare it with multiple worker
counts using the same inputs and seed. Test conservation invariants (population,
resources, and unique locations) as systems are added. Profile release builds
before parallelizing a phase; the executor itself does not make a phase faster
or race-free.
