![image](https://github.com/user-attachments/assets/39fd0f50-88a5-4f9e-9342-9ea6bf911c9e)


# What is Zeytin ?

**Zeytin** is a lightweight and modular game engine written in **C++**, built on top of [Raylib](https://github.com/raysan5/raylib). It features a fully integrated editor built using [Dear ImGui](https://github.com/ocornut/imgui), offering a streamlined and efficient development workflow.

# Features

-  Lightweight, modular core engine
-  Editor developed as a separate C++ application
-  Component-based design (similar to Unity)
-  Advanced runtime type information with automatic code generation
-  Cross-platform build system powered by [Premake](https://premake.github.io/)

# Prerequisites

## Windows
- MinGW-w64 with GCC compiler (tested on version 14.2.0)
- Premake5
- Git

## Linux
- Clang compiler (tested on version 14.0.0)
- Premake5
- Git

Required development libraries:

```bash
sudo apt-get install libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libasound2-dev libpulse-dev libdw-dev libbfd-dev libdwarf-dev libzmq3-dev
```

## Editor Dependencies

- ImGui - Immediate mode GUI library for the editor interface  
- Raylib - Graphics and input handling library  
- ImGui-Test-Engine - Testing framework for ImGui-based applications  
- ZeroMQ (ZMQ) - Messaging library for communication with the engine  
- RapidJSON - JSON parsing and generation library  
- rlImGui - Integration layer between Raylib and ImGui  

## Engine Dependencies

- Raylib - Graphics and input handling library  
- RTTR - Run-time type reflection library for C++  
- ZeroMQ (ZMQ) - Messaging library for communication with the editor  
- RapidJSON - JSON parsing and generation library  
- Tracy - Profiling library (used in debug builds, currently Linux only)  
- Backward-cpp - Stack trace library (Linux only)  

All of these dependencies are included in the repository under the 3rdparty directories in both the editor and engine folders, so you don't need to install them separately. The build scripts will use these local versions.

## Installation Steps

### 1. Clone the Repository

```bash
git clone https://github.com/berkaysahiin/Zeytin.git  
cd zeytin
```

### 2. Setting up the Editor

```bash
cd editor  
premake5 gmake  
cd build  
make
```

After building, run the editor:

```bash
cd ..  
./bin/Debug/ZeytinEditor
```

### 3. Setting up the Engine

Editor mode:

```bash
cd engine  
premake5 gmake  
cd build  
make config=EDITOR_MODE
```

Standalone mode:

```bash
cd engine  
premake5 gmake  
cd build  
make config=STANDALONE
```

### 4. Running the Engine

The engine will automatically launch from the editor using the "Start Engine" button.

To run the standalone engine manually:

```bash
cd engine  
./bin/STANDALONE/Zeytin
```

## Troubleshooting

### Windows
- If you encounter DLL errors when running the editor, ensure libzmq-mt-4_3_5.dll is in the correct path (targer directory, where the executable is located)
- Ensure the MinGW bin directory is in your PATH environment variable. (and double check the version)

### Linux
- For missing library errors, make sure all required development packages are installed.
- For ZMQ-related errors, verify that libzmq3-dev is properly installed.

---

# A Tour of Zeytin

Upon launching the editor, you'll find an intuitive interface designed for ease of use:

## Hierarchy Panel (Left)
  - Displays all entities and their components in the current scene.
  
## Console (Bottom)
  - Outputs real-time log messages from both the engine and the editor.
  
## Asset Browser (Right)
  - Lets you navigate and manage project files and folders.

![Zeytin Editor UI](https://github.com/user-attachments/assets/67f2b4a7-10bb-40cd-bc7b-3be628679f8d)

## Running the Engine

- You can design and edit levels directly in the editor—no need to start the engine immediately.
- To start the engine:
  - Click the **"Start Engine"** button at the top of the editor.
  - This will automatically **build** and **run** the engine.

# Your First Game: Adding a Moving Cube

In Zeytin, the engine and editor are separate executables, but the game code is part of the engine project. This means:

- The **game code is compiled with the engine code**.
- The **parser expects all game-related code** to be placed under a specific directory: 

## Project Structure

```plaintext
zeytin/
├── editor/                 
│   ├── include/            
│   ├── source/             
│   └── build.sh            
├── engine/                 
│   ├── include/
│   │   └── game/           ← Game header files
│   ├── source/
│   │   └── game/           ← Game implementation files
│   └── build.sh            
└── shared_resources/       
    ├── entities/           
    └── variants/           
```

## Creating a Variant

In the **Zeytin** ecosystem, components are referred to as **variants**. This naming convention originated from the internal implementation dependency on `rttr::variant`, and the term has persisted ever since.

### Step 1: Define a Simple `Position` Variant


```plaintext
zeytin/
├── editor/                 
│   ├── include/            
│   ├── source/             
│   └── build.sh            
├── engine/                 
│   ├── include/
│   │   └── game/
            └── position.h              ← Add here   
│   ├── source/
│   │   └── game/          
│   └── build.sh            
└── shared_resources/       
    ├── entities/           
    └── variants/           
```


### Key Concepts

- **Variants** are a core abstraction in Zeytin used to define game object components.
- **Macros** like `VARIANT()` and `PROPERTY()` are essential for reflection and parsing:
  - `VARIANT(TypeName)`:
    - Generates boilerplate code necessary for the engine to recognize and manage the variant.
  - `PROPERTY()`:
    - An empty macro used by the editor/parser to identify fields that should be treated as editable properties.

### Example: `position.h`

```cpp
#pragma once

#include "variant/variant_base.h"

class Position : public VariantBase {
    VARIANT(Position)  // Required macro for engine-side registration

public:
    Position(float x, float y) : x(x), y(y) {}

    // Public member variables with PROPERTY() macro
    float x = 0; PROPERTY()  // Marks this field as an editable property
    float y = 0; PROPERTY()
};
```

### Example: `speed.h`

Now let's define a speed variant, under ```game/include```:

```cpp
#pragma once

#include "variant/variant_base.h"

class Speed : public VariantBase { 
    VARIANT(Speed);

public:
    float value; PROPERTY();
};

```


### Example: `cube.h`

Finally, let's a cube variant where we will also have the logic, under ```game/include```:

```cpp
#pragma once

#include "variant/variant_base.h"
#include "game/position.h"

class Cube : public VariantBase {
    VARIANT(Cube);

public:
    float width = 50.0f; PROPERTY()
    float height = 50.0f; PROPERTY()
    Color color = RED; PROPERTY() // Nesting is also OK, Color is a Raylib struct.
    
    void on_init() override; // Lifetime method, called right after construction
    void on_update() override; // Lifetime method, called every frame
    void on_play_update() override; // Lifetime method, called every frame while in play mode

    /*  Not overriden ones:
        void on_post_init() override; // Called after on_init()
        void on_play_start() override; // Called on entering play mode
        void on_play_late_start() override // Called after on_play_start()
    */
    
private:
    void handle_input();
};
```

Now it's time to code some logic, let's add cube.cpp under ```source/game/```

```cpp
#include "game/cube.h"
#include "game/speed.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"

#include "remote_logger/remote_logger.h" // To send log messages to the editor console

void Cube::on_init() {
    // Initialization if needed
    log_info() << "Cube is initialized" << std::endl;
}

void Cube::on_update() {
    // Get the position component
    auto& position = Query::get<Position>(this);
    
    // Draw the cube
    draw_rectangle(
        position.x - width / 2,
        position.y - height / 2,
        width,
        height,
        color);
}

void Cube::on_play_update() {
    // Handle user input during play mode
    handle_input();
}

void Cube::handle_input() {
    auto& position = Query::get<Position>(this);
    // Using Query::read for Speed as it's read-only
    const auto& speed = Query::read<Speed>(this);
    
    float delta_time = get_frame_time();
    float movement_speed = speed.value;
    
    // Handle keyboard input for movement
    if (is_key_down(KEY_LEFT) || is_key_down(KEY_A)) {
        position.x -= movement_speed * delta_time;
    }

    if (is_key_down(KEY_RIGHT) || is_key_down(KEY_D)) {
        position.x += movement_speed * delta_time;
    }

    if (is_key_down(KEY_UP) || is_key_down(KEY_W)) {
        position.y -= movement_speed * delta_time;
    }

    if (is_key_down(KEY_DOWN) || is_key_down(KEY_S)) {
        position.y += movement_speed * delta_time;
    }
}
```

## Creating a Design-time Entity 

Now, let's create a design-time entity. I am referring this as **design-time** because this entity will be created within the editor and constructed upon start by the engine as it loads the scene, as opposed to **runtime-created** entities, which refers to entities that are instantiated while the game is running.

* To create a design-time entity, click on "Create New Entity" and enter a name.

![image](https://github.com/user-attachments/assets/c4716a26-e451-42ee-bccb-30e4b9ee50e5)


## Putting it all together

Now that we have an entity, let's start the engine, so it will compile and build the engine and let the engine generate **variant** files necessary for the editor.

* Right click on **TestEntity** and add **Cube** variant to the entity.
* To help with the setup and minimize setup related issues, ZeytinEditor auto adds queried variants. Queried **Speed** and **Position** variants are automatically added once the Cube variant is added.
* You can disable this feature and (optionally) enforce variants by hand:


```cpp
#pragma once

#include "variant/variant_base.h"
#include "game/position.h"

class Cube : public VariantBase {
    VARIANT(Cube);
    IGNORE_QUERIES(); // Editor will not introduce any query based dependency check for this variant
      // optionally:
    REQUIRES(Position, Scale); // You can still enforce input variants as requirements by hand.
    // This is OK even if the query based dependency check is enabled, still parsed and still enforced

public:
    float width = 50.0f; PROPERTY()
    float height = 50.0f; PROPERTY()
    Color color = RED; PROPERTY()

    void on_init() override; 
    void on_update() override; 
    void on_play_update() override; 

private:
    void handle_input();
};
```


* You can tweak property values with engine on/off
* I have increased the size a bit, set the speed value to a non-zero value, and set the start position
* Enter play mode, and move the cube as you want
* You can tweak values while in play mode, but the changes are not saved
* Exit play mode to return to the original scene

![image](https://github.com/user-attachments/assets/db955d5a-cd96-4bc7-a31e-4085d17ea024)


# Query API

The Query namespace provides a convenient way to work with entities and their variants in the Zeytin engine.

## Key Functions

- `create_entity()`: Creates a new entity and returns its ID.
- `has<T>(entity_id)`: Checks if an entity has a specific variant type.
- `get<T>(entity_id)`: Gets a reference to a variant for modification.
- `read<T>(entity_id)`: Gets a const reference to a variant for read-only access.
- `try_find_first<T>()`: Finds the first entity with a specific variant type.
- `find_first<T>()`: Like try_find_first but throws if not found.
- `find_all<T>()`: Returns all variants of a specified type.
- `find_all_with<T, U...>()`: Returns all entity IDs that have the specified variant types.
- `find_where<T>(predicate)`: Returns variants matching a predicate function.
- `for_each<T>(action)`: Executes an action on all variants of a type.
- `add<T>(entity_id, args...)`: Adds a variant to an entity, optionally with constructor args.
- `remove_variant_from<T>(entity_id)`: Removes a variant from an entity.
- `remove_entity(entity_id)`: Removes an entity completely.

## Example Usage

```cpp
// Create an entity with position and sprite variants
auto entity_id = Query::create_entity();
Query::add<Position>(entity_id, 100.0f, 200.0f);
auto& sprite = Query::add<Sprite>(entity_id)->get();
sprite.path_to_sprite = "path/to/image.png";

// Retrieve and modify variants
if (Query::has<Position>(entity_id)) {
    auto& pos = Query::get<Position>(entity_id);
    pos.x += 10.0f;
}

// Work with multiple components
auto [position, velocity] = Query::get<Position, Velocity>(entity_id);
position.x += velocity.x * delta_time;

// Find entities with specific variants
for (auto id : Query::find_all_with<Position, Sprite>()) {
    // Process entities with both Position and Sprite
}

// Execute an action on all variants of a type
Query::for_each<Ball>([](Ball& ball) {
    ball.reset();
});

// Get multiple variants in one call
auto [position, sprite, collider] = Query::get<Position, Sprite, Collider>(entity_id);

// Read-only access
const auto [position, velocity] = Query::read<Position, Velocity>(entity_id);

// Safely handling variant queries
if (auto health_opt = Query::try_get<Health>(entity_id)) {
    Health& health = health_opt->get();
    health.value -= damage;
}

```

# SET_CALLBACK

The `SET_CALLBACK` macro provides a way to define callback methods that are automatically invoked when property values change in your variants.

## Usage

- Add the `SET_CALLBACK` macro after the property declaration  
- Implement the callback method in your variant class  
- The callback will be called whenever the property is modified through the editor or with the property update system  

Example:
```cpp
class Sprite : public VariantBase {  
    VARIANT(Sprite);

public:  
    // Declare property with a callback  
    std::string path_to_sprite; PROPERTY() SET_CALLBACK(handle_new_path);

    void on_init() override {  
        load_sprite_if_needed();  
    }

    // Implement the callback method  
    void handle_new_path() {  
        load_sprite_if_needed();  
        log_info() << "Sprite path changed to: " << path_to_sprite << std::endl;  
    }

private:  
    void load_sprite_if_needed() {  
        if (!path_to_sprite.empty()) {  
            texture = LoadTexture(path_to_sprite.c_str());  
            m_texture_loaded = true;  
        }  
    }

    Texture texture;  
    bool m_texture_loaded = false;  
};
```



