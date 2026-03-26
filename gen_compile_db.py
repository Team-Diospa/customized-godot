import json
import os

root = os.getcwd()
files = [
    "animation_system.cpp", "audio_system.cpp", "ecs_command_buffer.cpp",
    "ecs_prefab_bridge.cpp", "ecs_scheduler.cpp", "ecs_serializer.cpp",
    "entity_manager.cpp", "hierarchy_system.cpp", "input_buffer_system.cpp",
    "physics_system.cpp", "physics_system_2d.cpp", "register_types.cpp",
    "rendering_system.cpp", "rendering_system_2d.cpp", "shader_data_system.cpp"
]

db = []
common_flags = [
    "-std=c++17", "-fno-exceptions", "-DDEBUG_ENABLED", "-DDEV_ENABLED", "-DTOOLS_ENABLED",
    "-I" + root,
    "-I" + os.path.join(root, "core"),
    "-I" + os.path.join(root, "servers"),
    "-I" + os.path.join(root, "scene"),
    "-I" + os.path.join(root, "editor"),
    "-I" + os.path.join(root, "drivers"),
    "-I" + os.path.join(root, "platform/windows"),
    "-I" + os.path.join(root, "modules"),
    "-I" + os.path.join(root, "modules/ecs_core")
]

for f in files:
    full_path = os.path.join(root, "modules", "ecs_core", f)
    db.append({
        "directory": root,
        "arguments": ["clang++"] + common_flags + ["-c", full_path, "-o", full_path + ".o"],
        "file": full_path
    })

with open("compile_commands.json", "w") as f:
    json.dump(db, f, indent=2)

print("Generated compile_commands.json for modules/ecs_core")
