## Prompt 1
```
Create a C++20 project called Feature Serving Engine.

Requirements:
- use CMake
- generate synthetic user events
- support event types:
  - view
  - click
  - purchase
- ingest events continuously
- maintain an in-memory feature store:
  user_id -> feature vector

Feature vector fields:
- views
- clicks
- purchases

Update features incrementally as events arrive.

Print current feature state periodically to stdout.

Project structure:
- cli/
- core/events/
- core/features/
- core/store/

Constraints:
- stdlib only
- no external dependencies
- deterministic event processing
- must compile and run immediately
```

## Prompt 2
```
Extend the Real-Time Feature Serving Engine.

Add:
- producer thread for synthetic event generation
- thread-safe event queue
- consumer thread for feature aggregation

Requirements:
- use mutex + condition_variable
- support graceful shutdown
- preserve deterministic feature updates

Add metrics:
- events processed per second
- queue size

Create:
- core/concurrency/
- core/queue/

Do not remove existing feature computation logic.
```

## Prompt 3

```
Extend the system with a low-latency retrieval layer.

Requirements:
- add feature retrieval API:
  get_features(user_id)
- support concurrent reads
- benchmark retrieval latency

Output metrics:
- average retrieval latency
- p50 latency
- p99 latency

Create:
- core/retrieval/

Do not introduce networking yet.
Keep retrieval local/in-process.
```