### Description
This project is a functional two-pass compiler designed to process C-style syntax. It implements a complete front-end pipeline, including a lexical analyzer built with **Flex** and a syntax parser built with **Yacc/Bison**.

### Key Features

*   **Lexical Analysis:** Tokenizes C-style keywords, operators, and identifiers using Flex.
*   **Syntax & Semantic Analysis:** Uses a context-free grammar to parse variable declarations, function definitions, and control structures (`if-else`, `for`, `while`).
*   **Hierarchical Scope Management:** Implements a robust Symbol Table with a nested `scope_table` architecture to manage variable visibility and lifetime across different blocks.
*   **Abstract Syntax Tree (AST):** Constructs a comprehensive AST using C++ class definitions to represent the program structure.
*   **Intermediate Code Generation:** Generates **Three-Address Code (TAC)** from the AST for further optimization or execution.
