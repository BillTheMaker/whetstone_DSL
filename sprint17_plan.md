# Sprint 17 Plan: Language Batch 2 — .NET Family + SQL

## Context

Full-stack transpilation needs server-side languages and database layers.
Sprint 17 adds F#, VB.NET, and SQL dialects. This covers the Microsoft
ecosystem and the data layer that every real project touches.

**After Sprint 17:** 17 language parsers and generators.

---

## Phase 17a: F# Parser + Generator (Steps 405-409)

F# is a functional-first .NET language. ML-family syntax with type inference,
pattern matching, discriminated unions, and computation expressions. Tests
whether the AST handles ML-style functional programming (complements the
Lisp-style coverage from Sprint 14).

### Step 405: F# Parser (12 tests)
- `let name params = body` → Function
- `let mutable x = 5` → Variable (isMutable)
- `type Name = | Case1 | Case2 of int` → EnumDeclaration (discriminated union)
- `type Name = { field: Type }` → ClassDeclaration (record)
- `match x with | pattern -> expr` → pattern match → IfStatement chain
- `async { ... }` → AsyncFunction with computation expression
- `module Name` → NamespaceDeclaration
- `|> pipeline |> operators` → chained FunctionCalls
- Significant whitespace handling (indentation-sensitive)

### Step 406: F# Generator (12 tests)
- Extends ProjectionGenerator + SemannoAnnotationImpl
- F# conventions: `let`, pipe operators, pattern matching
- Type mapping: int, float, string, bool, unit, list, seq, option, result
- `commentPrefix() -> "// "`

### Step 407: VB.NET Parser (12 tests)
- `Sub Name(params) ... End Sub` → Function (void return)
- `Function Name(params) As Type ... End Function` → Function
- `Class Name ... End Class` → ClassDeclaration
- `Interface Name ... End Interface` → InterfaceDeclaration
- `Module Name ... End Module` → NamespaceDeclaration
- `Dim x As Integer = 5` → Variable
- `If...Then...ElseIf...Else...End If` → IfStatement
- `For Each x In collection ... Next` → ForStatement

### Step 408: VB.NET Generator (12 tests)
- VB syntax: `Sub`/`Function`, `Dim`, `End` blocks
- Type mapping: Integer, Double, String, Boolean, Object
- Case-insensitive output conventions
- `commentPrefix() -> "' "`

### Step 409: .NET Integration (8 tests)
- Cross-language: C# ↔ F# ↔ VB.NET (all .NET, high fidelity)
- Cross-language: Python → F# (OOP → functional)
- Pipeline.run() for both new languages
- All 3 .NET languages share type system annotations

---

## Phase 17b: SQL Dialect Support (Steps 410-416)

SQL is fundamentally different from all other languages: it's declarative,
set-based, and schema-oriented. The AST needs table/column/query representations.
Supporting SQL means Whetstone can reason about the full stack: frontend →
backend → database.

### Step 410: SQL AST Nodes (12 tests)
- New nodes: TableDeclaration, ColumnDefinition, SelectQuery, InsertStatement,
  UpdateStatement, DeleteStatement, JoinClause, WhereClause, IndexDefinition
- These are new node types specific to SQL — not mappable to existing Function/Class

### Step 411: PostgreSQL Parser (12 tests)
- CREATE TABLE, SELECT, INSERT, UPDATE, DELETE, JOIN, WHERE
- PostgreSQL-specific: SERIAL, JSONB, array types, DO blocks, PL/pgSQL functions
- Schema.Table notation

### Step 412: PostgreSQL Generator (12 tests)
- Generate valid PostgreSQL DDL and DML
- Type mapping to PostgreSQL types
- `commentPrefix() -> "-- "`

### Step 413: T-SQL Parser + Generator (12 tests)
- Microsoft SQL Server dialect: IDENTITY, TOP, NVARCHAR, stored procedures
- T-SQL-specific: BEGIN/END blocks, DECLARE, SET, PRINT
- Differences from PostgreSQL highlighted in annotations

### Step 414: MySQL Parser + Generator (12 tests)
- MySQL dialect: AUTO_INCREMENT, ENGINE=InnoDB, backtick quoting
- MySQL-specific: LIMIT syntax, GROUP_CONCAT

### Step 415: SQL Annotation Mapping (12 tests)
- @Risk(high) on DELETE without WHERE, DROP TABLE
- @Complexity from JOIN depth and subquery nesting
- @BoundsCheck from LIMIT/TOP presence
- @Contract(pre="index exists") for queries depending on indexes
- Cross-stack: Python ORM class ↔ SQL table definition

### Step 416: Phase 17b Integration + Sprint Summary (8 tests)
- 17 languages total (14 + F# + VB.NET + SQL counts as 1 with 3 dialects)
- Cross-stack: Python class → C# class → SQL table → migrations
- Annotation inference on SQL (risk detection, complexity)
- Sprint 17 totals verification

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 17a | 405-409 | 56 | F# + VB.NET parsers, generators, .NET integration |
| 17b | 410-416 | 80 | SQL AST nodes, PostgreSQL, T-SQL, MySQL, annotations |
| **Total** | **405-416** | **~136** | 12 steps |
