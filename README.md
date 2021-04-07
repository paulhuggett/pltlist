# pltlist

A tiny demo program showing how I intend to implement generation of PLT (and GOT?) sections in the llvm-project-prepo rld linker.

It’s important that rld guarantees identical outputs regardless of thread behaviour. This repo is proof-of-concept for a technique to do that quickly for the GOT/PLT. In this case the contents of the section are determined by fixups rather than definitions; we encounter those fixups in essentially random order (because the compilations are scanned concurrently). The code doesn’t introduce any egregious locking in the process (just a couple of atomic variables).
