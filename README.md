# Lunar
The simple lexer generator for a more civilized era.

## What is Lunar?
Lunar is a lexer generator that creates simple lexers. It was originally created for syntax highlighting in the Flare text editor.
It does not use regular expressions, but does use pattern matching.

## Why is it different from Flex?
Using Flex, you put code directly in your parser definition file. Makes sense for a tool from the '70s. But in the modern era, if
function call overhead is what is slowing down your code, you are writing some very fast code indeed.

Lunar can also be used as a static library to generate parsers from C/C++. This is how Flare uses it, rather than interpreting a
file. In fact, Lunar generates its own file parser this way!

Lunar uses callbacks, which are supplied by the calling application.

## What can it do?
Lunar is specifically designed to make lexers for regular languages and syntaxes. It recognizes two constructs, keywords and spans.

A keyword is simply a string that is matched. A span is a length of text that starts with a certain string and ends with a certain
string. An example of a keyword would be "const" or "int" in C/C++. A span would be block comments (`/*` to `*/`) or one-line
comments (`//` to a newline character) in C/C++.

When Lunar finds a match, it performs the callback you supplied, providing the location and length of the match.

A notable feature of Lunar is that it can be used on Lunar files (.lun) to generate a parser, or the code generator can be supplied
a parser tree (see Lunar.h) to generate a parser. Examples of the latter are how Flare generates its parser, and Lunar itself.
