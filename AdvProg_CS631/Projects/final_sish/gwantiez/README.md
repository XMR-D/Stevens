Comments about submission structure :

At first the way my project is structure can appear complex and unnecessary
But I wanted to implement a shell with a modular approach, that is I design
the structure so that every bit of code is compartmented, especially the 
builtins. That way if for any reasons (even if it will not happen) we want
to add a new builtin, then we just need to add it within the builtins repo
update the builtin makefile, add the module path in the main Makefile
and increase the BUILTIN_NB value in generic_mkflags.

the exit builtin is a special one since it's directly impacting the shell main
process, for this reason it is not implem as a separate module but rather
directly inside the shell code.

The compilation process has also been specifically designed for this reason
and expliciltly state which builtin is compiled in which order. Normally
first all the builtins are compiled then the shell sources and finally 
everything is linked together.

Another aproach would have been to compile the builtins as shared libraries
but it would have been overkill and not really adapted to the "Built-in" 
fashion.



Problems encountered : 

- Adapting GNU Make way of doing things to BSD Make was harder than anticipated

- Taking arbitrary decision on the limitations of each builins and the way
the shell behaved, was tricky (should quotes need to be taking into account,
should pipeline with an open | at the end should produce an error or ask the
user to finish typing a command....) 
    => I considered that since it's a simple sh implementation, the behavior
       should be simple as well, so every case that was not describe in the man
       page (open pipes, etc...) produced an error.
    => For the builtins, I tried to obtain a behavior as close as possible to
       the real implementation of sh() but limiting the supported special cases
       to the sish man page description. 
       (i.e nothing in the sish man page sated that '.' should be a delimiter for
        the environment variable, for example in $VAR.GARBAGE only $VAR would
        be expanded in the sh() implem)

- Several realloc bugs in the parsing logic that were hard to understand that
  forced me to change chunks of the logic to find a way around. 
  
  A particular pointer aliasing bug that was really intresting : 
  When the parser called realloc to add new tokens into a subcommand, 
  sometimes the underlying allocator was considering the last allocated pointer 
  returned by realloc free, forcing the malloc library to reuse the pointer 
  due to optimizations, overwriting the previous token by the new one.
  
  I checked the code and no calls to free were made at anytime except at the
  end after the logging function that showed the bug. I also encounter similar
  issues with the internals allocations made by strdup() (that also used realloc).

  To solve it I just stopped using realloc as much as I could and replace it by 
  a less optimized logic that will first compute the number of element that needs
  to be allocated and then use calloc or malloc once.

  I have the feeling there is something shady in the realloc implem on NetBSD
  since I tested the snippet of code on my Linux distro and worked just fine
  it can be an optimization I don't know about or something else, but it
  was interesting to dive in how this bug occured (learned a few tricks on gdb)

- The rest went smoothly, once the parser where adapted to handle pipes and
  redirections it was a matter of executing the pipeline (which was just like
  the command assignement with slight changes to handle multiple threads 
  that all executed one subcommand, setting up the pipes beforehand, opening
  the redirections paths to overwrite the pipes fds and catching the status codes).
