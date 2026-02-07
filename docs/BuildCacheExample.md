# Building a Cache - Whetstone AST (Canonical Form)

```
Function: getOrCreate
  @{MemoryAnnotation}  // One of: @Deallocate(Explicit), @Lifetime(RAII), @Reclaim(Tracing), @Owner(Single), etc.
  Parameter: cache -> Map<string, Widget>
  Parameter: key -> string
  Body:
    IfStatement:
      condition: key in cache
      then: Return cache[key]
      else:
        Assignment: widget = Widget.create(key)
        Assignment: cache[key] = widget
        Return: widget
```