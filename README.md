# SpacetimeUE++

**Schema-powered multiplayer for Unreal Engine.**  
*Live logic, reflective state, and algebraic structure—from SpacetimeDB into Blueprints.*

---

## 🚧 Prototype Status

> ⚠️ This plugin is in **Prototype** phase and has only been tested against the [`quickstart-chat`](https://spacetimedb.com/docs/modules/c-sharp/quickstart) SpacetimeDB tutorial database.  
> Expect bugs, missing features, and breaking changes. Contributions and feedback welcome!

---

## ✨ Overview

**SpacetimeUE++ is not just an integration—it’s a proposition.**  
What if your game logic wasn’t duct-taped across RPCs, state machines, and database calls—but declared once, semantically, and made fully available inside the Engine?

SpacetimeUE++ connects Unreal Engine with [SpacetimeDB](https://spacetimedb.com), delivering **real-time schema reflection**, **Blueprint-ready reducer bindings**, and **semantic typing** via SATS (Semantic Algebraic Type System).

> **SpacetimeDB is the next level of Reflection in Unreal.**  
> Not just metadata—but a living, authoritative system woven into your gameplay layer.

> **Building an alternate Verse requires boldness and ambition.**  
> One thus needs more than technology and services: **one needs to define new language**.

With automatic type generation, native Blueprint access, and optional REST/gRPC protocol support, **SpacetimeUE++ transforms your SpacetimeDB schema into gameplay code**—clean, declarative, multiplayer-ready, and deeply Unreal-native.

This is more than middleware. It’s a **philosophy of simulation**—made ergonomic.

---

> _This plugin is part of my personal prototype to explore and extend SpacetimeDB’s Unreal integration.  
If you're from the team: I’m already building with you in mind._

---

### 🧠 Beyond Integration: Co-Authoring Logic

While SpacetimeDB is designed for seamless integration, its true power is unlocked through **intentional reducer design**.

SpacetimeUE++ is not just a way to expose reducers in Blueprint—it’s a prototype for understanding how Unreal Engine and SpacetimeDB can **co-author gameplay logic**.

Is the best path emitting reducer code from Blueprints, compiling to WebAssembly, and deploying to the DB?  
Or is it designing gameplay systems that **converse fluently** with SpacetimeDB's reducer architecture—*speaking its language*, not just invoking its methods?

We’re here to explore both.

> Because client-side logic doesn’t breathe a unified world.  
> The world’s logic **belongs to the world**—not to its agents.

When logic is fragmented across clients, **reality splinters**. There is no consistent causality, no shared truth. The result? Desyncs, exploits, chaos.

If the logic of the world is to feel **coherent, authoritative, and scalable**, it must live in a system designed to host it. That’s SpacetimeDB’s domain.

And if you're wondering why that matters—ask the Metaverse.  
Zuckerberg spent **$10 billion** trying to simulate a shared world.  
He didn’t get close.

---

### 🌍 Why the Hassle?

Because this isn’t about patching replication.  
It’s not about smoothing over UE networking quirks.  
It’s not even about fast prototyping.

It’s about building a **Verse**.

A persistent, consistent, scalable world demands more than replicated variables and client-authoritative hacks. It requires:

- **One logic**  
- **One timeline**  
- **One source of truth**

And it lives **server-side**.

> Client-side logic doesn’t breathe a coherent world.  
> The world’s logic belongs to the world—not to its agents.

When logic is fragmented across clients, causality breaks.  
State becomes subjective. Synchronization becomes guesswork.  
And if you think that’s enough to simulate 100,000 players—or a million—good luck.

Zuckerberg spent **$10 billion** chasing that dream.  
He didn’t get close.

We go through the hassle because we want to.  
Because it's the **only honest path** to large-scale shared reality.

Because **this is the work required** to make the Verse *real*.

---

## 🔁 What It Does

| Feature | Description |
|--------|-------------|
| 🔧 Codegen | Generates `USTRUCT`s for tables and `UFUNCTION`s for reducers from SpacetimeDB schemas |
| 🧠 SATS Semantic Mapping | Respects SpacetimeDB’s notion of types fundamentally as algebraic objects—Products with Terms, Sums with Variants, Optionals, and Lists.
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

<!--
## 🧠 Philosophy

> "Building an alternate Verse requires boldness and ambition.  
> One thus needs more than technology and services: one needs language."

SpacetimeUE++ is that language.
It turns schema into ontology.
It turns reducers into verbs.
It makes Unreal **speak** your world.

This is not mere codegen. It’s **ontological synchronization**—gameplay logic shaped by algebraic meaning.
-->
