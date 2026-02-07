# Processing a List of Records - Whetstone AST (Canonical Form)

```
Function: processRecords
  @{MemoryAnnotation}  // One of: @Deallocate(Explicit), @Lifetime(RAII), @Reclaim(Tracing), @Owner(Single), etc.
  Parameter: records -> List<Record>
  Body:
    ForLoop:
      iterator: record in records
      body:
        Call: record.validate()
        Call: record.transform()
    Return: records
```