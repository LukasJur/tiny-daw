# C++ crash course (for a Java/C-background dev)

Written while starting the Tiny DAW project (JUCE + CMake). Priority-ordered by how soon each concept shows up in the codebase, anchored to Java/C knowledge.

## 1. Stack vs. heap & value semantics (the big one)

In Java, every object created with `new` lives on the heap, and every variable holding it is a *reference* — assignment copies the reference, not the object. There is no way to have an object "directly," only handles to one.

C++ has both. `MainComponent mc;` puts an actual `MainComponent` object on the stack — no heap, no pointer, it just *is* the object, and it's destroyed automatically when the enclosing scope ends. `MainComponent* mc = new MainComponent();` is the Java-like heap version, but now *you* must call `delete` or it leaks forever — C++ has no GC. Assigning one object to another (`a = b;`) by default **copies the whole thing**, field by field — nothing like Java's reference-copy unless you're dealing with a pointer or reference type.

This is why `setContentOwned(new MainComponent(), true)` explicitly heap-allocates — `DocumentWindow` needs to own an object whose lifetime outlives the function call that created it, which a stack object can't do.

## 2. RAII & destructors

Since there's no GC, C++'s cleanup discipline is: a destructor runs automatically the instant an object's scope ends (stack) or `delete` is called (heap) — deterministically, unlike Java's `finalize`/GC which run "eventually, maybe." This is why `shutdownAudio()` in `MainComponent`'s destructor matters — it forces a synchronization point (audio thread guaranteed done) *before* member teardown begins, because C++ gives you no such guarantee automatically.

Related gotcha: during a destructor's execution, the object is in a partially-torn-down state (members destruct in reverse declaration order after the destructor body runs, then the base class destructor runs), and virtual calls made during that window no longer dispatch to derived overrides. Java has no equivalent because GC only reclaims an object after nothing can possibly be calling into it.

## 3. References vs. pointers

C gave you pointers (`int*` — can be null, can be reassigned, need `*`/`->` to dereference). C++ adds **references** (`int&`) — an alias for an existing variable that can't be null and can't be reseated after creation. Example: `getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)` — `bufferToFill` isn't a copy, it's an alias for the caller's actual object, but reads like a plain variable (no `->`).

Rule of thumb: reference when something can't be null and you're not transferring ownership; pointer (or smart pointer) when it can be absent or ownership matters.

## 4. Smart pointers (replaces "just let the GC handle it")

`std::unique_ptr<MainWindow> mainWindow;` is C++'s answer to "heap object, no GC": it owns the object and automatically calls `delete` when the `unique_ptr` itself goes out of scope — RAII applied to heap memory. Critically, a `unique_ptr` **cannot be copied**, only *moved* (ownership transfers, the source becomes empty) — enforcing "exactly one owner" at compile time, which Java's shared-reference model doesn't need because GC doesn't care how many references exist.

`std::shared_ptr` is the reference-counted alternative for genuine shared ownership, but it has real cost (see the Q&A below).

## 5. `const` correctness

Java's `final` only stops reassignment of the variable itself. C++'s `const` is pervasive: `const juce::String&` means "you may read this, you may not modify it," and `void foo() const` on a method means "this method promises not to modify the object." JUCE signatures use `const` constantly — it's part of the contract, and the compiler enforces it.

## 6. Header/source split & the compilation model

Java: one `.java` file per class, compiler resolves everything via the classpath. C++: a `.h` (declares "here's what exists") and a `.cpp` (defines "here's how it works") are traditionally separate, and each `.cpp` compiles independently into a `.obj` — the compiler processes files one at a time and knows nothing about other files except what you `#include`. This is why `#include` matters so much and why a build system like CMake exists at all (Java has no equivalent build-orchestration need). JUCE modules bend this slightly (single-header modules), but the underlying model is the same.

`#include` is a preprocessor directive that literally pastes the target file's text in at that point, before real compilation even starts. That's why every header you write needs `#pragma once` as its first line: without it, if the same header ends up `#include`d twice in one compiled file (directly, or transitively through two other headers that both include it), its contents get pasted in twice — and the compiler sees the same class defined twice, which is an error. `#pragma once` tells the compiler "only paste this file's contents once per compiled file, no matter how many `#include`s point at it." (The fully-standard equivalent, predating `#pragma once`, is an `#ifndef`/`#define`/`#endif` include guard — same idea, more typing, no risk of relying on a non-standard-but-universally-supported pragma. Either is fine in practice.) `.cpp` files never need this, since they're compiled standalone and aren't themselves `#include`d by anything.

## 7. Virtual functions, `override`, abstract classes

Java methods are virtual (dynamically dispatched) by default. C++ methods are **not** — you must write `virtual` for a method to be overridable via a base-class pointer/reference, and `override` (like `@Override`) has the compiler verify you're actually overriding something (e.g. `void paint (juce::Graphics& g) override`). A pure virtual method (`virtual void foo() = 0;`) is C++'s abstract method — makes the class non-instantiable, exactly like Java's `abstract`.

## 8. Templates (generics, but compiled differently)

Java generics are erased at compile time (one bytecode implementation, casts inserted). C++ templates are **stamped out per type at compile time** — `AudioBuffer<float>` and a hypothetical `AudioBuffer<double>` are literally two different generated classes. This shows up constantly (`juce::dsp::IIR::Filter<float>`, etc.); errors inside template code can produce long, ugly compiler errors — the actual mistake is usually one line in, don't panic at the wall of text.

## 9. Undefined behavior (no safety net — new vs. both Java and C)

Java: out-of-bounds access always throws `ArrayIndexOutOfBoundsException`. C: mostly the same category of "you'll probably segfault." C++ is stricter in a scarier way: many mistakes (out-of-bounds access, use-after-free, calling a virtual method mid-destructor) are **undefined behavior** — the compiler is allowed to assume you never do this, and if you do anyway, the result isn't "crash," it can be "appears to work, corrupts something unrelated three minutes later." There's no exception, no guaranteed crash. This is why the real-time audio rules (no allocation/locking on the audio thread) and destructor-ordering rules matter so much — the compiler won't catch violations for you.

---

## Q&A

**Q: Smart pointers exist — why not make every pointer a smart pointer?**

Smart pointers model *ownership*, not *usage*. Most pointers in a real codebase aren't about who's responsible for deleting something — they're just "here's a thing, go look at it, you don't own it." Examples: `getParentComponent()` returns a pointer to a `Component` that's owned by the component hierarchy, not by whoever calls the function. Wrapping every such pointer in a smart pointer would either be semantically wrong (implying ownership/shared ownership that doesn't exist) or force awkward workarounds (`unique_ptr` can't even be copied, so you couldn't have two non-owning views of one without immediately breaking uniqueness — you'd end up calling `.get()` to get a raw pointer anyway, right back where you started).

There's also real cost: `shared_ptr` uses atomic reference counting (thread-safe increment/decrement on every copy) plus a heap-allocated control block — overhead you never want on the audio thread (ties back to Q1: no allocation, and atomic ops add contention). `unique_ptr` is close to free, but its non-copyable nature means it's only right for genuine single-ownership, not for casual "let me pass this around to look at."

The practical convention (and JUCE's own style): use `unique_ptr`/`shared_ptr` at the *one place* that decides an object's lifetime, and plain pointers or references everywhere else that merely *uses* the object without owning it. The raw pointer itself is a useful signal — "I'm just borrowing this" — and erasing that distinction by smart-pointering everything would make ownership harder to reason about, not easier.
