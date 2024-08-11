# Similarity Analysis Of C programs for array dependencies
 Similarity Analysis of C programs using lex and yacc

The starting point of this project is comparing the codes written by the students with a
reference solution provided by educator. The comparison process should be extended beyond
the syntax to include structural and semantic similarities as well. This analysis should include
array structures


The project analyzes syntax and structural elements of codes written in C language using
lexer (lexical analyzer) and parser (semantic analyzer) tools. This analysis provides the basis
for semantic processing of code at later stages. With the data obtained after the parsing
process, Abstract Syntax Tree (AST) is created, which provides the structural representation
of the program. 

These structures reflect the problem-solving approach and algorithmic effectiveness of
student codes.

In the functional requirements section of the report, i inspect the essential capabilities and
behaviors of the system that are critical for meeting the intended requirements. The functional
requirements of my project are listed below:

• The system shall parse the code successfully.
• The system shall create the AST Nodes successfully.
• The system shall add the child nodes to parent nodes successfully.
• The system shall update the match table successfully.
• The system shall give the similarity of arrays in context of dimensions, types and indexes successfully
