# SpacetimeDB UE5 Integration

# SpacetimeUE++

*A complete Unreal Engine integration for SpacetimeDB: schema reflection, Blueprint access, direct protocol awareness, and a new semantic layer for game logic.*

---

## 🚧 Prototype Status

> ⚠️ This plugin is in **Prototype** phase and has only been tested against the [`quickstart-chat`](https://spacetimedb.com/docs/modules/c-sharp/quickstart) SpacetimeDB tutorial database.
> Expect bugs, missing features, and breaking changes. Contributions and feedback welcome!

---

## 🧠 Philosophy

SpacetimeDB, by its very name—and through its core currency, **Energy**—invokes the deepest ideas in physics:

- **Dynamics**: Systems evolve through clear rules.
- **Freedom**: Agents act and choose within those constraints.
- **Conservation**: Transactions are structured, reliable, and meaningful.

This integration doesn't just treat SpacetimeDB as a server backend. It **blends it into Unreal Engine**, introducing its **semantic worldview** into the engine's very fabric.

> We're not just wiring two technologies together.
> We're teaching Unreal to **speak the language of the living universe**—inside human-made machines.

---

## 🔤 SATS: The Semantic Algebraic Type System

This plugin adopts and extends SpacetimeDB’s `typespace` with a formal, algebraically grounded naming convention:

| SpacetimeDB Type | SATS Role | Unreal Mapping |
|------------------|-----------|----------------|
| `Product`        | `Terms`   | `USTRUCT` with fields |
| `Sum`            | `Variants`| Tagged union or manual discriminated struct |
| `Vector<T>`      | `List<T>` | `TArray` with semantic labels (`Children`, `Entries`, etc.) |
| `Option<T>`      | `Optional`| Nullable type with semantic safety |

With SATS enabled, generated code reflects not just structure—but **intent**.

```cpp
// Product with Terms
USTRUCT(BlueprintType)
struct FPosition {
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite)
  float X;

  UPROPERTY(BlueprintReadWrite)
  float Y;
};

## Prerequisites

* **Unreal Engine 5.5** (or later) installed
* **SpacetimeDB CLI** (`spacetime`) available on your system PATH, or bundled alongside your game
* **C++ development environment** (Rider, Visual Studio, etc.)

## Installation

### Option A: Git Submodule

```bash
cd <YourUEProjectRoot>
git submodule add https://github.com/jstreibel/Spacetime-UE.git Plugins/Spacetime-UE
```

### Option B: Clone Directly

```bash
cd <YourUEProjectRoot>/Plugins
git clone https://github.com/jstreibel/Spacetime-UE.git
```

1. From your project root, regenerate project files:

   ```bash
   ./GenerateProjectFiles.sh -platform=Linux  # or Windows/Mac as appropriate
   ```
2. Open the solution in an external editor, e.g. Rider (Linux), Visual Studio (Windows), VSCode (Multi-platform), XCode (MacOS), etc.
3. Build the solution.
4. In the Unreal Editor, go to **Edit → Plugins**, locate **SpacetimeCLI**, and enable it.
5. Restart the editor.

## Usage in Blueprints

1. In your Blueprint, add a **Call Function** node from **SpacetimeDB** category.
2. Example: **DescribeDatabase**

    * **Input**: Database name (string)
    * **Outputs**:

        * **Tables** string array
        * **Reducers** string array
        * **Error** message (string)
3. Use **Print String** or your UI to display returned values.

```blueprint
// Pseudocode:
DescribeDatabase("MyDB", Tables, Reducers, OutError);
PrintStringArray(Tables);
PrintStringArray(Reducers);
```

<!-- ## Versioning & Releases

We use [Semantic Versioning](https://semver.org/):

* **Major** versions introduce breaking changes
* **Minor** versions add functionality in a backward-compatible manner
* **Patch** versions make backward-compatible bug fixes

Tag releases on GitHub as `v<MAJOR>.<MINOR>.<PATCH>` and include a changelog in the release notes.-->

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature-name`)
3. Commit your changes (`git commit -m "Add feature"\`) and push (`git push origin feature-name`)
4. Open a Pull Request

Please follow the existing [code style](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine) and add tests where applicable.

## License

This project is licensed under the [MIT License](LICENSE).
