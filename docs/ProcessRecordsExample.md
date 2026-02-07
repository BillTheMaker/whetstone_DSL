# Processing a List of Records - Whetstone AST (Canonical Form)

```
Function: processRecords
  @deref(???)  // Strategy applied here
  Parameter: records -> List<Record>
  Body:
    ForLoop:
      iterator: record in records
      body:
        Call: record.validate()
        Call: record.transform()
    Return: records
```