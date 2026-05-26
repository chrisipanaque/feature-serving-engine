## Architecture

```
Event Stream
↓
Threaded Ingestion
↓
Parallel Feature Aggregation
↓
Feature Store
↓
Retrieval Service
↓
Inference Endpoint
↓
Latency Metrics
```

## Retrieval Latency (10,000 requests, 4 threads)

| Metric | Value |
|--------|-------|
| Average latency | 1.74 µs |
| P50 latency     | 0.42 µs |
| P99 latency     | 21.63 µs |