![Zeytin](https://github.com/user-attachments/assets/39fd0f50-88a5-4f9e-9342-9ea6bf911c9e)

# Zeytin

Zeytin is an experimental C++23 game engine built with Raylib and Dear ImGui. It includes a component-based runtime, an editor, automatic reflection metadata generation, and a bundled sample game.

The current supported development platform is Linux with Clang 22, CMake 4.2+, and Ninja.

## Build

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

For a standalone release:

```bash
cmake --preset standalone-release
cmake --build --preset standalone-release
```

Component registration code and editor metadata are generated automatically during the build.

## Screenshots

![Editor](https://github.com/user-attachments/assets/2bef44c3-292a-45d9-ae6c-4d7af7120510)

![Create entity](https://github.com/user-attachments/assets/c4716a26-e451-42ee-bccb-30e4b9ee50e5)

![Runtime editing](https://github.com/user-attachments/assets/db955d5a-cd96-4bc7-a31e-4085d17ea024)

![Tracy profiling](https://github.com/user-attachments/assets/c9383a40-90c0-4480-87ce-a6ef376ab4d2)

## Built With Zeytin

[Zone Control](https://berkaysahiin.itch.io/zone-control) was created in 48 hours for YILDIZ Game Jam.
