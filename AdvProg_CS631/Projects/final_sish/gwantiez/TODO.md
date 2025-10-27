# Priority 1 : Implement a state machine tokenizer + parser, that will create a
char * array, that will be walked through to evaluate the command passed.
Once done integrate exit, cd and echo builtins and test

# Priority 1 : Implement a data structure that will be able to handle
tokens that can vary (redirections, control operators, cmd body, cmd options and background operator)

# Priority 1: Fork exec commands using sh(1) or other ?? => Need to clarify this with teacher.

# Priority 2 : Add shell tests in README.md

# Priority 3 : Clean code + clang format

# TIP : I/O redirection must be resolved before anything else
        (i.e ls -R / > /dev/null &) 

