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