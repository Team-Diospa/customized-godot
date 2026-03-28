# ECS Core Demonstration Script
# This script shows how to integrate the ECS core into a standard Godot project.

extends Node

# 1. Preload your ECS-compatible prefabs
const ENEMY_PREFAB = preload("res://prefabs/enemy.tscn")
const PROJECTILE_PREFAB = preload("res://prefabs/bullet.tscn")

var active_enemies = []

func _ready():
	print("Initializing ECS Demonstration...")
	
	# Register a high-level GDScript system to the ECS loop
	# This runs automatically after the ECS hierarchy is resolved.
	ECSScheduler.register_process_system(self._process_ecs_logic)
	
	# Spawn some entities
	for i in range(10):
		var id = ECSPrefabBridge.spawn_from_scene(ENEMY_PREFAB)
		var proxy = EntityManager.get_entity_proxy(id)
		
		# Set initial positions via the proxy
		proxy.transform_x = randf_range(-50, 50)
		proxy.transform_z = randf_range(-50, 50)
		proxy.debug_label = "Enemy_" + str(i)
		
		active_enemies.append(id)

func _process(_delta):
	# Example: Telemetry monitoring
	var stats = ECSScheduler.get_detailed_stats()
	if Engine.get_frames_drawn() % 60 == 0:
		print("ECS Entities: ", stats.entity_count)
		print("ECS Frame Time: ", stats.frame_time_usec, "us")

func _process_ecs_logic():
	# 2. Bulk Processing in GDScript
	# Find all entities with transforms and audio components
	var mask = EntityManager.BIT_TRANSFORM | EntityManager.BIT_AUDIO
	var targets = EntityManager.get_entities_with_mask(mask)
	
	for id in targets:
		var proxy = EntityManager.get_entity_proxy(id)
		
		# Example logic: Rotate everyone with an audio component
		proxy.transform_rot_y += 0.05
		
		# Example logic: Conditional destruction
		if proxy.transform_y < -10.0:
			print("Entity ", id, " fell out of world. Destroying.")
			EntityManager.destroy_entity(id)

func _on_fire_projectile(pos: Vector3, dir: Vector3):
	# 3. Spawning from scene with parenting
	var id = ECSPrefabBridge.spawn_from_scene(PROJECTILE_PREFAB)
	var proxy = EntityManager.get_entity_proxy(id)
	
	proxy.transform_x = pos.x
	proxy.transform_y = pos.y
	proxy.transform_z = pos.z
	
	# Attach components manually if needed (not in prefab)
	# EntityManager.add_component(id, ...)
