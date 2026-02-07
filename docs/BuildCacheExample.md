# Building a Cache - Whetstone AST (Canonical Form)

```
Function: getOrCreate
  @deref(???)
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