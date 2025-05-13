# SpacetimeDB UE5 Integration

A lightweight Unreal Engine 5.5 plugin that wraps the SpacetimeDB CLI (`spacetime`) to provide Blueprint-accessible functions for querying and describing databases.

## Prerequisites

* **Unreal Engine 5.5** (or later) installed
* **SpacetimeDB CLI** (`spacetime`) available on your system PATH, or bundled alongside your game
* **C++ development environment** (Rider, Visual Studio, etc.)

## Installation

### Option A: Git Submodule

```bash
cd <YourUEProjectRoot>
git submodule add https://github.com/jstreibel/SpacetimeCLI.git Plugins/SpacetimeCLI
```

### Option B: Clone Directly

```bash
cd <YourUEProjectRoot>/Plugins
git clone https://github.com/jstreibel/SpacetimeCLI.git
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
