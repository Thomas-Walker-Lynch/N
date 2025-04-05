# N: Experimental Multi-Precision Library and Language

This project is an experimental system for building a high-performance multi-precision arithmetic library in C, with no headers, using a powerful template and namespacing mechanism based on the C preprocessor.

## Highlights

### 🧠 Headerless Development in C
All modules follow a two-part structure: interface and implementation, guarded by `#ifndef Module·FACE` and `#ifdef Module·IMPLEMENTATION`. This allows all code to be included in any order without needing `.h` files.

### 🧰 Advanced CPP Extensions
The `cpp_ext` system introduces macro-based logic (e.g., `IF`, `FIND`, `CAT`, `EVAL`, `WHILE`), enabling token-based templating, membership testing, and extensible list manipulation—all within standard C.

### 🧬 Parametric Polymorphism via Templates
Modules such as the `TM` (tape machine / iterator) library are implemented generically via macro substitution (`_TM·CVT_`). This enables instantiations like `TM·AU`, `TM·Str`, and more, while preserving type safety and zero runtime overhead.

### 🌀 Namespaces in C
All public symbols use the `·` (U+00B7 MIDDLE DOT) to simulate hierarchical namespacing (e.g., `TM·AU·Array·init_pe`). This avoids naming collisions and allows clean introspection and tooling.

### 🧪 Iterator Framework Demonstrated
The `TM` system defines abstract iterators ("tape machines") over bounded or unbounded data. Implementations include:
- `Array` (classic memory-backed segment)
- `ZeroLength` (null iterator)
- `SingletonSegment` (one-value, writable)
- `SingletonCircle` (one-value, looped)
These implementations verify polymorphic dispatch, step/read/write behavior, and status handling across modes.

### ❌ No Python Filter Required
Previous builds used a Python script for C preprocessing. This has been replaced by the native `cpp_ext.c` layer, making the system fully self-contained in C.

---

## 🚧 Still in Progress

- The core multi-precision arithmetic (`MP` types and operations) is under active development.
- Template-based `map`, `copy`, and reduction operators will be added next, leveraging the iterator interface.
- Future work includes algebraic dispatch, symbolic manipulation, and seamless CLI test generators.

