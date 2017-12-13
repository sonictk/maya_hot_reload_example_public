# Getting started #

## What are we even doing? ##

Here's [what's going down](https://www.youtube.com/watch?v=rFz59Jmgewk). The
normal approach for how a custom node in Maya would work would be something like
the following (connections that are called once are dashed, and connections that
are called all the time are solid):

{% dot high_level_overview.svg

digraph overview {
    graph[fontname="Arial", fontsize=14, nodesep=1, rankdir=LR];
    node[fontname="Arial", fontsize=14, shape=box];
    edge[fontname="Arial", fontsize=10];

    Maya[label="Maya"];

    host[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="limegreen"><b>Plugin DLL/SO</b></td></tr>
                <tr><td port="ip" border="1"><font point-size="10.0" face="Courier">initializePlugin()</font></td></tr>
                <tr><td port="up" border="1"><font point-size="10.0" face="Courier">uninitializePlugin()</font></td></tr>
                <tr><td port="d" border="1"><font point-size="10.0" face="Courier">deform()</font></td></tr>
                <tr><td port="o" border="1"><font point-size="10.0" face="Courier">...</font></td></tr>
            </table>
        >
    ];

    host:ip -> Maya[label="Called when plugin is loaded", style=dashed];
    host:up -> Maya[label="Called when plugin is unloaded", style=dashed];
    host:d -> Maya[label="Called when node is dirtied"];
}

%}

It's pretty straightforward; the plugin **Dynamic-link library (Windows/OSX) or
Shared Object (Linux)** contains all the initialization/uninitialization/deformation
logic/whatever necessary for the plugin to function; Maya just loads the entire binary,
calls the functions as needed whenever it determines it should, and that's the
end of the story.

What we're going to be doing instead is something like the following:

{% dot high_level_overview_1.svg

digraph overview {
    graph[fontname="Arial", fontsize=14, nodesep=1, rankdir=LR, size=9];
    node[fontname="Arial", fontsize=14, shape=box];
    edge[fontname="Arial", fontsize=10];

    Maya[label="Maya"];

    host[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="limegreen"><b>Host Plugin</b></td></tr>
                <tr><td port="ip" border="1"><font point-size="10.0" face="Courier">initializePlugin()</font></td></tr>
                <tr><td port="up" border="1"><font point-size="10.0" face="Courier">uninitializePlugin()</font></td></tr>
                <tr><td port="d" border="1"><font point-size="10.0" face="Courier">deform()</font></td></tr>
            </table>
        >
    ];

    client[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="yellow"><b>Client Plugin</b></td></tr>
                <tr><td port="l" border="1"><font point-size="10.0" face="Courier">logic()</font></td></tr>
                <tr><td port="o" border="1"><font point-size="10.0" face="Courier">...</font></td></tr>
            </table>
        >
    ];

    host:ip -> Maya[label="Called on load", style=dashed];
    host:up -> Maya[label="Called on unload", style=dashed];
    host:d -> Maya[label="Called when dirtied"];

    client:l -> host:d[label="Check last modified timestamp"];
}

%}

!!! note
    If the above didn't make any sense, please take a moment to review
    the
    [Autodesk documentation](http://help.autodesk.com/view/MAYAUL/2017/ENU/?guid=__files_Command_plugins_initializePlugin_htm)
    regarding how Maya plugins and their entry points work.

In this scenario, the ``logic()`` function will now contain all the **business
logic** necessary for the ``deform()`` function to work; the ``deform()``
function itself will become pretty much nothing but a wrapper around Maya's
machinery. (For simplicity's sake, I will not be discussing taking the
``deform()`` function out of the host plugin as well in this tutorial, though it
is certainly possible, and I encourage you to try!)

OK, so far it's just a level of abstraction that we've introduced here. What
happens when we re-compile the client plugin?

{% dot high_level_overview_2.svg

digraph overview {
    graph[fontname="Arial", fontsize=14, nodesep=1, rankdir=LR, size=9];
    node[fontname="Arial", fontsize=14, shape=box];
    edge[fontname="Arial", fontsize=10];

    Maya[label="Maya"];

    host[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="limegreen"><b>Host Plugin</b></td></tr>
                <tr><td port="ip" border="1"><font point-size="10.0" face="Courier">initializePlugin()</font></td></tr>
                <tr><td port="up" border="1"><font point-size="10.0" face="Courier">uninitializePlugin()</font></td></tr>
                <tr><td port="d" border="1"><font point-size="10.0" face="Courier">deform()</font></td></tr>
            </table>
        >
    ];

    clientOld[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="grey"><b>Client Plugin (Old)</b></td></tr>
                <tr><td port="l" border="1"><font point-size="10.0" face="Courier">logic()</font></td></tr>
                <tr><td port="o" border="1"><font point-size="10.0" face="Courier">...</font></td></tr>
            </table>
        >
    ];

    clientNew[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="yellow"><b>Client Plugin (New)</b></td></tr>
                <tr><td port="l" border="1"><font point-size="10.0" face="Courier">logic()</font></td></tr>
                <tr><td port="o" border="1"><font point-size="10.0" face="Courier">...</font></td></tr>
            </table>
        >
    ];

    host:ip -> Maya[label="Called on load", style=dashed];
    host:up -> Maya[label="Called on unload", style=dashed];
    host:d -> Maya[label="Called when dirtied"];

    host:d -> clientOld:l[label="1 .Unloads library", color=red, style=dashed, fontcolor=red];
    clientNew -> host:d[label="2. Loads library with newer modified
    timestamp", style=dashed];
    clientOld:l -> clientNew:l[label="3. Pointer re-directed to new address", style=dashed];
}

%}

This might seem a little confusing, so let's go through step-by-step:

* In the ``deform()`` function, we're checking the current modification
  timestamp of the **client plugin**. If we detect that the current file's
  timestamp is newer than the current one we have loaded in memory, we **unload
  the old one** and **load the new one into memory** instead.
* We then **fix up the function pointer** to the old ``logic()`` method to
  point to the address of the new ``logic()`` function in memory. If this
  doesn't make sense, I'll go over it in a bit. Basically, think of it as us
  telling the host plugin that the old ``logic()`` function is no longer valid
  and that it should be looking at our new one instead that we just modified.

!!! note "Function Pointers: the short version"
    If you're new to C/C++, you might not be familiar with what they are.
    **Function pointers** are basically, as the name implies, *pointers to
    functions*; they point to addresses of where the start of function calls
    live in memory. They allow us to basically re-direct our code to call newer
    versions of functions that we create on-the-fly as required.

    I recommend [getting familar with them](http://www.geeksforgeeks.org/function-pointer-in-c/)
    before continuing with this tutorial, as the concept is integral to this entire
    workflow.


## Understanding how libraries work ##

Before we continue any further, let's take a moment to recap on how libraries
work in operating systems, especially since there are some important differences
between how Windows/Linux/OSX handles them.

!!! warning
    I am not an expert on how these things work by any stretch of the
    imagination, and I highly encourage you
    to
    [read up on the subject](https://en.wikipedia.org/wiki/Dynamic-link_library)
    on your own as well to get a better understanding of the machinery
    involved. I will only cover what I think is important in understanding what
    we'll be attempting to accomplish within the scope of this project.

### Understanding the executable ###

You know what an executable is; it's the thing you basically double-click on in
your file explorer to run a program. When you do that, the OS basically does
something similar to the following (this is a grossly over-simplified
explanation of what is actually happening, but we'll gloss over that for now):

{% dot exe_overview.svg

digraph overview {
    graph[fontname="Arial", fontsize=14, nodesep=1, size=9, rankdir=LR];
    node[fontname="Arial", fontsize=14, shape=box];
    edge[fontname="Arial", fontsize=10];

    exe[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="limegreen"><b>.exe file (PE format)</b></td></tr>
                <tr><td port="h" border="1"><font point-size="10.0">header</font></td></tr>
                <tr><td port="i" border="1"><font point-size="10.0">intel code</font></td></tr>
                <tr><td port="f" border="1"><font point-size="10.0">fixup table</font></td></tr>
            </table>
        >
    ];

    memory[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="yellow"><b>Memory (RAM)</b></td></tr>
                <tr><td port="i" border="1"><font point-size="10.0">intel code</font></td></tr>
                <tr><td port="f" border="1"><font point-size="10.0">function calls</font></td></tr>
            </table>
        >
    ];

    windowsCode[label="Windows code" color="blue"];

    linker[label="Dynamic linker"];

    execution[label="Code runs"];

    exe:f -> linker[color="green"];
    linker -> memory:f[color="green", label="patches over"];
    exe:i -> linker -> memory:i[color="red"];
    windowsCode -> linker[label="e.g. WinMain()", color="green"];

    memory:i -> execution;
}

%}

The file format on Windows for such file is known as
the [Portable Executable](https://en.wikipedia.org/wiki/Portable_Executable)
format, which is basically a standard layout for how a binary file should look
like so that the OS (in this case, Windows) knows how to look inside it and find
the stuff it needs.

!!! tip "Crossing the platforms"
    On Linux/OSX, the file format used is known as the [Executable and Linkable
    Format (ELF)](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format).
    While not precisely the same as the Windows PE format, for our purposes, we
    can assume that they function in a similar manner.

Your ``.exe`` file that you compile contains various **memory segments**. There
is the **data section**, the **code section**, **stack** and the **fixup table**
(also known as the **import address table**). In it, there are assembly
instructions that make up the function calls that you've created in your source
code. These assembly instructions were generated when you compiled your source
code by whichever compiler you chose to use.

When you click on that ``.exe`` file, Windows creates a new process for
you and maps the executable into that process' memory. Permissions are set for
various sections of the data read in (i.e. the **code section** is set to executable,
the **data** section is read/write, and the **stack/constants** are
read-only). It then looks at the **fixup table** that you have in your
executable to see where your Windows/CRT function calls should be *patched over*
in memory to point to the actual address of Windows functions (i.e. **not**
inside your executable!) So things like ``strlen()``, ``WinMain()`` and even
things like ``malloc()`` all get executed this way.

### Understanding the library ###

OK, whatever. So what has this got to do with libraries?

First of all, let's get one thing straight: **a library is, for all intents and
purposes, exactly the same thing as an executable**. The major difference is
that the library is not directly executable (as the name implies), and a library
(usually) doesn't need to define an entry point. Libraries basically contain the
same things that an executable does; in fact, on Windows, the **Dynamic Link
Library (DLL)** format (which is known as a *shared library* format) uses the
exact same PE format that an ``.exe`` file does. The same is true for Linux as well,
where **Shared Object (SO)** and executables on Linux both share the ELF format.

One other difference is that typically, on Windows, a shared library *exports*
specific symbols for usage outside of itself through something called an
[export table](https://docs.microsoft.com/en-us/cpp/build/exporting-from-a-dll).
This exports table is what allows other executables like ``Maya.exe`` to be able
to access the functions such as ``initializePlugin`` and ``uninitalizePlugin``
in your ``.mll`` files (If you recall, you have to externalize these functions when
compiling your Maya plug-in as well!).

!!! tip "Crossing the platforms"
    On Linux/OSX, **all symbols in a ``.so`` file are available to an
    interrogating process (i.e. Maya) by default**. This means that we are able
    to lookup such symbols without the need for an export table unless we strip
    out the symbols manually; we'll talk about this later on during
    implementation of our compilation scripts.

    On this point, it's important to note that if it wasn't apparent,
    the ``.mll`` and ``.dll`` extensions are essentially the same; Maya just
    wants a ``.mll`` extension for convention's sake; the file formats are
    identical for both. On Linux, you should be aware that the convention for
    both a Maya plug-in extension and a normal shared library is ``.so``.

There are different types of libraries used for different purposes: **static**
libraries, which are libraries used during *linking* and actually combined into
the final executable, and **dynamic** libraries, which are libraries that, as
the name implies, *dynamically loaded* either at **load-time** or **run-time**
rather than actually being combined into the main executable file.

We'll be focusing on dynamic link libraries in this tutorial, since they give us the
properties we'll need for code hotloading. If you'd like to learn more about the
differences between the two and what they're each used for, I recommend [reading
up](http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html) on
the [subject](https://msdn.microsoft.com/en-us/library/windows/desktop/ms681938(v=vs.85).aspx).

In addition to dynamic linking, there are two ways to perform this as well, known as
[load-time](https://msdn.microsoft.com/en-us/library/windows/desktop/ms684184(v=vs.85).aspx)
dynamic linking, and
[run-time](https://msdn.microsoft.com/en-us/library/windows/desktop/ms685090%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396)
dynamic linking. For our purposes, we will be using **run-time** dynamic
linking, which basically means that we will manage loading the DLL ourselves at
*run-time*, reading that DLL's available symbols, and mapping **function pointers**
to point to those symbols.

## How it all comes together ##

So when all is said and done, here's what things look like when we introduce a
DLL to the mix:

{% dot altogether_now.svg

digraph overview {
    graph[fontname="Arial", fontsize=14];
    node[fontname="Arial", fontsize=14, shape=box];
    edge[fontname="Arial", fontsize=10];

    exe[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="limegreen"><b>Maya.exe file (PE format)</b></td></tr>
                <tr><td port="h" border="1"><font point-size="10.0">header</font></td></tr>
                <tr><td port="i" border="1"><font point-size="10.0">intel code</font></td></tr>
                <tr><td port="f" border="1"><font point-size="10.0">fixup table</font></td></tr>
            </table>
        >
    ];

    dll[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="limegreen"><b>.DLL file (PE format)</b></td></tr>
                <tr><td port="h" border="1"><font point-size="10.0">header</font></td></tr>
                <tr><td port="i" border="1"><font point-size="10.0">intel code</font></td></tr>
                <tr><td port="f" border="1"><font point-size="10.0">fixup table</font></td></tr>
                <tr><td port="et" border="1" bgcolor="lightblue"><font point-size="10.0">exports table</font></td></tr>
            </table>
        >
    ];

    memory[shape=none,
         label=<
            <table border="0" cellspacing="0">
                <tr><td border="1" bgcolor="yellow"><b>Memory (RAM)</b></td></tr>
                <tr><td port="i" border="1"><font point-size="10.0">intel code</font></td></tr>
                <tr><td port="f" border="1"><font point-size="10.0">function calls</font></td></tr>
            </table>
        >
    ];

    windowsCode[label="Windows code" color="blue"];

    linker[label="Dynamic linker"];

    execution[label="Maya runs"];

    exe:f:e -> linker[color="green"];
    dll -> exe[style="dashed", label="    Tells OS to load the library into memory"];
    dll:i -> linker[color="red"];
    dll:f:e -> linker[color="green"];
    linker -> memory:f[color="green", label="    patches over", fontcolor="green"];
    exe:i:e -> linker -> memory:i[color="red"];
    dll:et:s -> exe[color="lightblue", label="    reads addresses of functions", fontcolor="blue"];
    windowsCode -> linker[label="    Calls here could be from either the DLL or Maya", color="green", fontcolor="green"];

    memory:i:e -> execution:w;
}

%}

*Sorry, GraphViz (which is what I use to generate these diagrams) is a little
difficult to wrangle into producing a nice shape*.

Basically, we see that things aren't that different; all that really happens is
that Maya will now load in our ``.dll`` library, read the addresses of functions
from the exports table, presumably do whatever it needs to do (i.e. call
``deform()`` or ``compute()``, for example), which then in turn the dynamic
linker will patch over with calls from Windows functions (if they happen to be
used in those function calls), before finally executing the code from memory and
thus performing the work we'd like (i.e. the mesh deforms, the animation plays,
the entire thing crashes etc.)

!!! tip
    Casey Muratori did a great job of giving a more Windows-specific generic
    overview of this entire topic on the Handmade Hero stream; I would highly
    recommend watching it as well as he talks about other details that are of
    interest, such as how memory paging works, along with the **Virtual Memory
    Address System (VMAS)**, and the differences between *physical* and
    *virtual* memory. There are other things to learn, such as **Address Space
    Layout Randomization** (ASLR), and all the other minutae related to those
    concepts that we won't be covering within the scope of this tutorial.

    The video is available [here](https://hero.handmade.network/episode/intro-to-c/day5).

Hopefully all of the above made some sense! Next, we'll finally get to start
writing some code to start to get this implemented, and in the process,
hopefully learn a lot more about how memory works in general!.
