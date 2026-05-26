# Feature Serving Engine

Designed a real-time feature serving engine sustaining **145 events/sec** throughput with **sub-microsecond retrieval latency** and **9.6M ev/s inference throughput** using a 16-shard concurrent feature store backed by `std::shared_mutex` and batched event ingestion.

Architected a **thread-safe ingestion pipeline** with lock-free event queues, configurable batch processing (up to 64 events per batch), and append-only binary event log for deterministic crash recovery.

Built a **concurrent TCP serving layer** on standalone ASIO supporting `GET_FEATURES` and `PREDICT` over persistent connections, integrating sharded in-memory storage with a lightweight linear recommendation scorer.

## Usage

```
# Build
cmake -B build
cmake --build build

# Default run (TCP :8080, batch size 32)
./build/bin/feature-serving-engine

# Custom port and batch size
./build/bin/feature-serving-engine --port 9090 --batch 64

# Replay from persisted log
./build/bin/feature-serving-engine --replay events.log

# Run benchmarks
./build/bin/benchmarks                       # throughput + latency
./build/bin/benchmarks --throughput          # events/sec only
./build/bin/benchmarks --latency             # retrieval + inference latency
```

## Architecture

### System Topology

```
┌──────────────────────────────────────────────────────────────────────────┐
│                        Feature Serving Engine                            │
│                                                                          │
│  ┌──────────┐   ┌──────────────────┐   ┌────────────────────────────┐   │
│  │  Event    │   │ ThreadSafeQueue  │   │       Consumer              │   │
│  │ Generator ├──▶│   (FIFO)        ├──▶│  ┌──────────────────────┐  │   │
│  │ (seeded   │   │   push/pop_batch│   │  │ batch=32             │──┼──▶│──▶ events.log
│  │  PRNG)    │   └──────────────────┘   │  │ store.ingest_batch() │  │   │
│  └──────────┘                           │  └──────────────────────┘  │   │
│                                         └──────────┬─────────────────┘   │
│                                                    ▼                     │
│                                          ┌──────────────────────┐       │
│                                          │    Feature Store      │       │
│                                          │  16 shards ×          │       │
│                                          │  shared_mutex per     │       │
│                                          └──┬───────────────┬────┘       │
│                                             │               │           │
│                                ┌────────────┘               └─────────┐ │
│                                ▼                                       ▼ │
│                     ┌────────────────────┐                  ┌──────────┐ │
│                     │  InferenceService  │                  │  Scorer  │ │
│                     │  predict(user_id)──┼──features──────▶│ (linear) │ │
│                     └────────┬───────────┘                  └──────────┘ │
│                              │                                            │
│                     ┌────────▼──────────┐                                 │
│                     │   TCP Server      │                                 │
│                     │   ASIO :8080      │                                 │
│                     │ thread-per-connect│                                 │
│                     └───────────────────┘                                 │
└──────────────────────────────────────────────────────────────────────────┘
```

### Feature Store — 16-Shard Concurrent Map

```
                           user_id
                              |
                         shard_index = user_id % 16
                              |
                              ▼
    ┌────────┬────────┬────────┬────────┬────────┬────────┬────────┬────────┐
    │ Shard0 │ Shard1 │ Shard2 │  ...   │  ...   │  ...   │ Shard14│ Shard15│
    ├────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┤
    │shared_ │shared_ │shared_ │ shared_│ shared_│ shared_│shared_ │shared_ │
    │ mutex  │ mutex  │ mutex  │  mutex │  mutex │  mutex │ mutex  │ mutex  │
    ├────────┴────────┴────────┴────────┴────────┴────────┴────────┴────────┤
    │              unordered_map<uint64_t, FeatureVector>                    │
    │                                                                        │
    │  Each value: FeatureVector { uint64_t views, clicks, purchases }       │
    └────────────────────────────────────────────────────────────────────────┘

  ┌─ ingest(event)   → unique_lock on 1 shard     → O(1)
  ├─ get_features(uid) → shared_lock on 1 shard   → O(1)
  └─ ingest_batch(events) → partition by shard,
                            1 unique_lock per shard → O(shards)
```

### Request Lifecycle

```
  Client                          Server (ASIO)
    │                                  │
    ├─── TCP connect ──────────────────▶│  acceptor.accept()
    │                                  │  std::thread(Session::run)
    │                                  │  thread.detach()
    │                                  │
    ├─── "GET_FEATURES 42\n" ─────────▶│  request.parse()
    │                                  │  store.get_features(42)
    │                                  │    ├─ shard = 42 % 16 = 10
    │                                  │    ├─ shared_lock(shard[10].mtx)
    │                                  │    └─ return {12, 4, 1}
    │                                  │
    │◀── "views=12 clicks=4 ───────────┤  response.serialize()
    │      purchases=1"                │
    │                                  │
    ├─── "PREDICT 42\n" ──────────────▶│  request.parse()
    │                                  │  service.predict(42)
    │                                  │    ├─ store.get_features(42)
    │                                  │    └─ scorer.score(fv)
    │                                  │       = 1.0*12 + 3.0*4 + 10.0*1
    │                                  │       = 34.0
    │                                  │
    │◀── "views=12 clicks=4 ───────────┤
    │      purchases=1 score=34.0"     │
    │                                  │
    ├─── disconnect ──────────────────▶│  socket closes, thread exits
```

### Threading & Lifecycle

```
  Startup                                        Runtime
  ───────                                        ──────
  main()                               ┌──────────────────────────────────┐
    │                                  │                                  │
    ├─ 1. Replay events.log ──────────▶│   Server      Producer   Consumer │
    │     → rebuild feature state      │   Thread      Thread      Thread │
    ├─ 2. Spawn Server thread          │   (accept)    (gen ev)  (ingest) │
    ├─ 3. Spawn Producer thread        │      │           │          │    │
    └─ 4. Spawn Consumer thread        │  per-conn     push     pop_batch │
                                       │  threads      (ev)      (32)    │
                                       │  (detach)      │          │     │
  Shutdown                              │    │           ▼          ▼     │
  ────────                              │    │    ┌─────────┐ ┌────────┐ │
  1. server.stop()                      │    │    │  Queue  │ │  Store │ │
  2. producer.stop()                    │    │    └─────────┘ └────────┘ │
  3. consumer.stop()                    └──────────────────────────────────┘
  4. final state + benchmarks
```

## Retrieval Latency (10,000 requests, 4 threads)

| Metric | Value |
|--------|-------|
| Average latency | 1.74 µs |
| P50 latency     | 0.42 µs |
| P99 latency     | 21.63 µs |
