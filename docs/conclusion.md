# Closing Thoughts #

## Applications ##

So, what else can this be used for other than deformers? Think about things like
Maya MPxCommands; GUI widgets (via hooking the ``QEventLoop`` instead of the
``deform``/``compute`` functions), or even scene callbacks! *The possibilities
are endless*...for abuse, that is. While the overhead of constantly checking of
an updated DLL is not significantly performance-draining, repeated checks across
several DLLs could have an effect.

In my opinion, within the context of Maya, it's best to utilize this technique
as a development aid, rather than something you leave enabled in shipping code,
controlled via a simple preprocessor definition or similar mechanism.

## Current Limitations ##

### State transfer ###

Right now, this little demonstration doesn't do anything to save any static
state across reloads of the DLL, nor does it handle adding new business logic
functions and making those hot-loadable as well; you must maintain a defined set
of entry points beforehand. There are several possible solutions to this;
[``cr.h``](https://fungos.github.io/blog/2017/11/20/cr.h-a-simple-c-hot-reload-header-only-library/#the-problem)
handles this quite elegantly, and I might decide to take a similar approach as
well in the future.

### Single-shot functions ###

Certain functions in Maya, such as ``initialize`` and ``creator`` are called
only once when the node is created; as such, if we wanted to do things like add
attributes to a node dynamically, we cannot utilize this existing approach of
keeping the update in the ``deform`` function; we'd need to find a different way
of not just making sure that the DLL is reloaded for that ``initialize``
function, but also to make sure that all nodes in the scene are updated to point
to that new ``initialize`` function as well, along with making any modifications
to memory that might entail. This is not a trivial problem to solve.

### Crash/Error handling ###

Right now, we don't do any sort of crash handling in the DLL at all; this means
that if we accidentally compile code that executes UB, we could end up crashing
Maya very easily. One thing to look into is to possibly catch such crash
signals, and if so, load a previous known good working version of the DLL and
attempt to continue execution. Again, ``cr.h`` has some implementation of this
work as well; I might take a stab at porting it over to a Maya implementation in
the future.

## Conclusion ##

Even with all the downsides and possible crashes, to me, being able to
continuously work on your code and watch it update live 5 out of 10 times is
better than having to unload/reload 10 out of 10 times; I've been utilizing this
approach recently when writing a deformer on my own project, and I'm definitely
far more productive than when I work conventionally, even with all my build
shortcuts and tests.

In summary, I hope you've learned a little more about memory and how it works
over the course of this tutorial, whether or not you choose to implement this
development technique in production! Feel free to contact me or file questions
on the repository page; I know I'm not the greatest when it comes to explaining stuff.
