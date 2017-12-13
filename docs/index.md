# Runtime Compiled C++ in Maya #

## About ##

This is a tutorial on how to setup a workflow for writing C++ code in Maya that
is **hot-loadable**; which is to say, you can compile your code and watch your
changes take effect *instantly, on-the-fly*, without having to unload/reload your
plugin, or worse, restart Maya.

All the source code for this example node is available
[here](https://bitbucket.org/sonictk/maya_hot_reload_example).


## Why should I care? ##

Several reasons:

* **Being able to iterate quickly is a receipe for success**. This applies in many
  disciplines, but especially so in programming; *the faster you see results, the
  faster you can make decisions about those results and continue with your
  work.* The less time you spend waiting between rebuilds/IDE lag/getting coffee,
  the more time you spend *making decisions about your data*.
  The more decisions you make, the more work you're able to get done.

* **People dislike C/C++ because it is perceived to be "difficult" or "slow to
  write in".** (In fact, I used to be like this as well when I first started out
  learning the language.) The alternative (which usually means using a scripting
  language like Python/MEL/Lua, or using an intermediate runtime solution
  through LLVM IR like [Fabric Engine](https://vimeo.com/101975842)/Maya
  Bifrost) is actually *slower* to iterate in when it comes to production use cases.
  Nevertheless, it must be recognized that when it comes to writing Maya
  plug-ins, it is, in truth, a very involved process for even an experienced C++
  programmer to even get a plug-in compiled and working for the first
  time. **This workflow aims to make that approach faster than any scripting
  language-based workflow can achieve.**

* **You can debug state-based problems much more easily.** For example, let's
  say you were writing a deformer, and had a bug that only occurred when the
  input mesh was deformed a certain way, on a specific frame, and perhaps only 1
  out of 5 times under the right circumstances. Worse, it's a simulation, so
  your output is not always idempotent (thus making it very difficult to unit
  test). Conventionally, you would need to unload/reload the plugin, re-setup
  the mesh in the specific manner that you suspect would trigger the issue and
  repeat this process ad-infinitum while you maybe had a debugger attached in
  the hope that you could catch the issue. With the ability to hotload your code
  on-the-fly, you can *see your decision changes immediately as you make changes
  to the code in context*. This makes it incredibly powerful as a debugging
  tool, since you can now easily narrow down the cause of bugs by changing the
  code, re-compiling, and watching your deformer *update instantly*, all without
  having to re-setup the context (mesh inputs, playback to current frame, etc.).

* **Programming becomes fun again!** Let's face it; the process for getting a
  Maya plugin working can be infinitely more draining than the process of
  actually writing the code itself; even without taking the Maya API itself into
  account with its ideas forced unto you, the build/linking process is incredibly
  convoluted, the dependencies number in the dozens, and the worst part is that
  all of it almost always has *nothing* to do with the actual problem you're
  trying to solve. If you could reduce the amount of *cruft* you have to deal
  with and focus on the **business logic** of the plug-in you're trying to
  *craft* instead, wouldn't you prefer that?


### Why not use a scripting language? ###

With all that being said, a very common solution to the problems above already
exists, and has for quite some time now: scripting languages. Whether it be
writing your code in Python/Lua/C#/MEL whatever, what usually happens is that
these languages have a **interpreter** embedded in the host application, and
allow you to write some code, perform Just-In-Time (JIT) compilation on it to
machine code to execute directly.

!!! note "What about LLVM Intermediate Runtime (IR)?"
    There are solutions that go further by compiling your source code to an
    [intermediate runtime](https://idea.popcount.org/2013-07-24-ir-is-better-than-assembly/)
    (the most popular method seems to be using
    the [LLVM toolchain](http://llvm.org/) to do so) before allowing the LLVM
    backend to translate it to actual CPU instructions, allowing
    for greater optimization by the compiler, thus resulting in improved performance
    over having an embedded interpreter .

    [Marco Giordano](https://github.com/giordi91/babycpp) has a great
    simplified example of this running using his own custom compiler for his
    implementation of his C-like scripting language inside Maya. **Fabric Engine**, as
    already mentioned, was taking advantage of this method as well, and the new
    **Bifrost** framework in Maya will also be utilizing this approach.

However, these solutions suffer from the same issue that all scripting languages
suffer from: they only give you what they've specifically exposed; you rarely
have full control over the actual memory you're dealing with, and in the case of
a Maya C++ plugin, you don't have full access to the entire Maya API (even if
you're using OpenMaya.) As we are *programmers*, our primary job is first and
foremost to **deal with memory**. Therefore, the existing solutions are what I
would consider *incomplete*, and thus we will not discuss them throughout this
document.

That being said, there is a time and place for such scripting languages and
techniques, and by no means should you view their usage as undesirable.


### How is this going to work? ###

Basically, the idea is that we will have a single ``.cpp`` file, ``logic.cpp``,
that will act as our *entry-point* "script" file of sorts; we'll be able to
continuously edit that file, re-build our plugin, and see our deformer update in
the Maya viewport.

We'll have two libraries, one that will be known as the **host library**, and
the other as the **client library**. The client library will be the one that is
continuously re-compiled and hot-loaded, while the host library will remain
(mostly) immutable. Of course, we should be able to add additional ``.cpp`` and
``.h`` files with their own corresponding logic as well and hot-load them as
necessary, but everything will go through that original ``logic.cpp`` file
first; that's why it's referred to as the *entry-point* for our *business
logic*.


## Requirements ##

### What you will need ###

- **Maya 2016 or higher**. (Really, any modern version of Maya will do.)
- A **C/C++ development environment** set up and ready to go. (If you want to see
  what my Emacs setup looks like, it's
  available [here](https://github.com/sonictk/lightweight-emacs).)
- Access to the [Linux manpages](https://linux.die.net/man/)/[Windows MSDN documentation](https://developer.microsoft.com/en-us/windows).
- Spare time and an open mind.

This tutorial will provide instructions for the main supported platforms that
Maya runs on, namely Windows, Linux and OSX. However, for fairly obvious reasons,
the tutorial will come from a *Windows-first approach*. Whenever platform-specific
information is appropriate, it will appear in the following form:

!!! tip "Crossing the platforms"
    Platform-specific information goes here.


### What you should know ###

- **Basic knowledge of C/C++**. I will focus on including only the code that is
  important; I expect you to be able to understand how to fill in the rest as
  needed. At the very least, you should be able to compile a Maya plug-in and
  run it using whatever toolchain you're familar with.

- Knowledge of how the dependency graph works in Maya and how dirty propagation works.

- **Basic knowledge of how Maya plugins work and how to write/build them**. There is
  a sample ``CMakeLists.txt`` build script provided for reference if you need a
  refresher on that. You can use whatever build system you want; I just use CMake
  for convenience's sake.

In the next section, we'll go over a high-level overview of what this tutorial is
attempting to cover and some basic fundamentals of how memory works.
