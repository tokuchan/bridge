\page page_ste_glossary STE Glossary

This page lists the technical names that ASD-STE100 lets a manual add
for its own equipment or domain. See
[ADR-0018](https://github.com/tokuchan/bridge/blob/master/docs/adr/0018-ste100-writing-standard.md)
for the full decision. Each entry has one approved meaning. Facility
pages and source comments use these words only with the meaning shown
here.

**A note on precision.** This page applies ASD-STE100's rules and
common vocabulary from public knowledge of the standard, not a copy of
the licensed document itself. Treat the word-level detail here as a
careful best effort, not a guarantee. Check a real copy of ASD-STE100
for anything that must be exactly right.

This page does not repeat terms that CONTEXT.md already defines
(`Truss`, `Deck`, `Rivets`, `Entity`, `Detector`, `Named Detector`,
`Feature Test`, and the namespace terms). Those terms keep their
CONTEXT.md meaning here too.

## General programming words

### class
A type that groups data and the actions on that data.

### struct
A class where every member is public by default. This page uses
"class" and "struct" with the same general meaning: a type that
groups data and the actions on that data.

### function
A named piece of code that does one action. You can give it input
data. It can give back a result.

### method
A function that belongs to a class.

### free function
A function that does not belong to a class.

### template
A pattern for a class or a function. You give the pattern a type. The
compiler makes the real class or function from the pattern and the
type.

### parameter
A named input that a function or a template accepts.

### argument
A value that you give to a function when you call it. The value fills
a parameter.

### callable
A function, or an object that you can call like a function.

### call
To make a function or a callable run.

### compile
To turn source code into a program that a computer can run.

### header
A file that contains C++ code. You can include a header in another
file.

### namespace
A named group of related code.

### inherit
To build a class from another class. The new class gets all the
actions and the data of the class that it inherits from.

### instance
One specific object that a class or a template makes.

### construct
To build a new instance of a class.

### chain
To call one method, then call another method on the result, in one
statement.

### value
A piece of data of a specific type. An `int` value and a `std::string`
value are examples.

### empty
Holds no value. An empty `optional` holds no value.

### result
The value that a function gives back when it finishes.

### return
To give back a value at the end of a function. The function's caller
gets this value.

### check
To look at something, to find one fact about it.

### macro
A named piece of text that the preprocessor replaces with other text,
before compiling. A macro can take arguments, like a function.

### expand
To replace a macro with the text it stands for. The preprocessor does
this before compiling.

### translation unit
One source file, plus every header it includes, after the
preprocessor expands every macro and directive in it. The compiler
compiles one translation unit at a time.

## Bridge-specific words

### polyfill
Code that gives you a function from a newer C++ standard. You can use
this code on an older standard that does not have the function.

### passthrough
Deck's choice to use the real C++ standard library function directly.
Deck makes this choice when the current compiler and library already
supply the function.

### ecosystem
One specific combination of a compiler and a standard library.
Different ecosystems can supply different C++ functions.

### comparator
A function or a macro that checks one version number against
another, and gives back true or false. `gt`, `ge`, `lt`, `le`, and
`eq` are bridge's five comparators.

### gate
To let code run only when a check passes. A Feature Test can gate a
passthrough choice.

### monadic method
A method on `optional` (or a similar type) that lets you chain
operations. Each method checks if the type holds a value. The method
does the operation only when a value is there.
