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