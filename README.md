# SpacetimeUE++

**From Schema to Semantics**  
*A SATS-powered, schema-first integration of SpacetimeDB into Unreal Engine.*

---

## ✨ Overview

**SpacetimeUE++** is not just a plugin. It’s a paradigm shift.  
It brings the full semantics of [SpacetimeDB](https://spacetimedb.com) into Unreal Engine—not just as an authoritative server backend, but as a **semantic layer**, an **algebraic model of gameplay logic**, and a **language** for real-time worlds.
> Building an alternate Verse requires boldness and ambition.
> One thus needs more than technology and services: **one needs to define new language**.

With automatic code generation, Blueprint-native reducer calls, SATS-powered type introspection, and optional protocol-level integration, SpacetimeUE++ transforms Unreal into a **declarative, multiplayer-first engine**—without sacrificing control or clarity.

---

## 🔁 What It Does

| Feature | Description |
|--------|-------------|
| 🔧 Codegen | Generates `USTRUCT`s for tables and `UFUNCTION`s for reducers from SpacetimeDB schemas |
| 🧠 SATS Semantic Mapping | Treats types as algebraic objects—`Products` with `Elements`, `Sums` with `Variants` and `Optionals`, and `Lists` |
| 🧩 Blueprint-Ready | Exposes reducer calls and database tables as native Blueprint nodes and types |
| 🛠️ Editor Integration | Live schema sync, diagnostics, and regeneration tools within the Unreal Editor |
| 🌐 Protocol Flexibility | Optional REST/gRPC integration for CLI-free deployment and live data wiring |
| 🔐 Auth Support | JWT-based authentication for secure, multi-tenant projects |
| 🧪 CI/CD Friendly | Dockerized build support, CLI automation (`stdb-ue`), and testable codegen pipelines |

---

## 🧬 SATS: Semantic Algebraic Type System

Unlike naive reflection layers, SpacetimeUE++ adopts the **SATS** approach to types:

| SpacetimeDB | SATS Concept | Unreal Output |
|-------------|---------------|----------------|
| `Product` | `Elements` of a composite type | `USTRUCT` with named `UPROPERTY`s |
| `Sum`     | `Variants` of an option space | Tagged `USTRUCT` or `TVariant` |
| `Vector`  | `List<T>` | `TArray<T>` |
| `Option`  | `Optional<T>` | Nullable wrapper, safe-by-construction |

SATS enforces a **semantic structure on your types**, aligning code with **intended meaning**, not just technical compatibility. This gives your game state the **expressive rigor of a language**.

---

## 🧠 Philosophy

> "Building an alternate Verse requires boldness and ambition.  
> One thus needs more than technology and services: one needs language."

SpacetimeUE++ is that language.
It turns schema into ontology.
It turns reducers into verbs.
It makes Unreal **speak** your world.

This is not mere codegen. It’s **ontological synchronization**—gameplay logic shaped by algebraic meaning.
