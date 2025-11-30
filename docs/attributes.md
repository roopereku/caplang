# Attributes

***NOTE: The description aims to illustrate the intent of the feature and not everything is implemented yet*** 

Caplang supports applying attributes to declarations, expressions and statements.

Attributes can be used to provide tooling and code executed at parse time additional context on what any given part of the source code. For example, if a declaration is intended to only exist in a debug context:
```
@debug
func someDebugFunction()
{
    // ...
}

func main()
{
    @debug
    someDebugFunction()
}
```
The `@debug` attribute can be used to specify that a declaration only exists in a debug context and a call to it is only executed in a debug context.

## Syntax

An attribute begins with `@` and contains an identifier or a function call:

```
// Examples of attributes
@foo
@debug
@someFunction("someValue", 10)
```

Multiple attributes can be chained together. All of them are attributed to the desired target:
```
@foo @debug @someFunction("someValue", 10)
func targetForAttributes1()
{
    // ...
}

@foo
@debug
@someFunction("someValue", 10)
type TargetForAttributes2
{
}
```