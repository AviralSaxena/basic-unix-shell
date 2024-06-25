# msh: A Simple Unix Shell

## Overview
This project was developed to build a simple Unix shell named `msh`. The main goals were to:
- Gain familiarity with the Linux environment.
- Understand the fundamentals of process management.
- Implement essential shell functionalities.

## Project Details

### What I Accomplished:
- **Command Line Interpreter**: Modified `msh.c` to handle command parsing and execution.
- **Execution Loop**:
  - Designed a loop to print a prompt.
  - Parsed user input.
  - Executed commands using `fork()`, `execv()`, and `wait()`.
- **Built-in Commands**:
  - Implemented `exit` and `cd` commands within the shell.
- **Output Redirection**:
  - Added functionality to redirect command outputs to different streams.
- **Compatibility**:
  - Ensured the shell works seamlessly in both interactive and batch modes.
- **Testing and Error Handling**:
  - Conducted thorough testing using provided scripts.
  - Implemented robust error handling mechanisms.

## Conclusion
This project enhanced my understanding of shell scripting and process management.