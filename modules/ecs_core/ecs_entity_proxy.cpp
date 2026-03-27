/**************************************************************************/
/*  ecs_entity_proxy.cpp                                                  */
/**************************************************************************/
#include "ecs_entity_proxy.h"

#include "entity_manager.h"

#include "core/object/class_db.h"

void ECSEntityProxy::set_entity(uint64_t p_id) {
	entity_id = p_id;
	notify_property_list_changed();
}

uint64_t ECSEntityProxy::get_entity() const {
	return entity_id;
}

bool ECSEntityProxy::_set(const StringName &p_name, const Variant &p_value) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em || !em->is_entity_valid(entity_id)) {
		return false;
	}

	String name = p_name;
	if (name.contains("/")) {
		String comp_name = name.get_slice("/", 0) + "Component";
		String prop_name = name.get_slice("/", 1);

		if (em->get_registry_untyped(comp_name) && em->get_registry_untyped(comp_name)->has(entity_id)) {
			Variant data = em->get_registry_untyped(comp_name)->get_untyped(entity_id);
			if (data.get_type() == Variant::DICTIONARY) {
				Dictionary d = data;
				d[prop_name] = p_value;
				em->update_component_untyped(entity_id, comp_name, d);
				return true;
			}
		}
	}

	return false;
}

bool ECSEntityProxy::_get(const StringName &p_name, Variant &r_ret) const {
	EntityManager *em = EntityManager::get_singleton();
	if (!em || !em->is_entity_valid(entity_id)) {
		return false;
	}

	String name = p_name;
	if (name == "Entity/ID") {
		r_ret = (Variant)entity_id;
		return true;
	}

	if (name.contains("/")) {
		String comp_name = name.get_slice("/", 0) + "Component";
		String prop_name = name.get_slice("/", 1);

		ISparseSet *set = em->get_registry_untyped(comp_name);
		if (set && set->has(entity_id)) {
			Variant data = set->get_untyped(entity_id);
			if (data.get_type() == Variant::DICTIONARY) {
				Dictionary d = data;
				if (d.has(prop_name)) {
					r_ret = d[prop_name];
					return true;
				}
			} else if (comp_name == "WorldTransformComponent") {
				// Special handling for read-only struct access
				const WorldTransformComponent &wc = em->get_component<WorldTransformComponent>(entity_id);
				if (prop_name == "x") { r_ret = wc.x; return true; }
				if (prop_name == "y") { r_ret = wc.y; return true; }
				if (prop_name == "z") { r_ret = wc.z; return true; }
			}
		}
	}

	return false;
}

void ECSEntityProxy::_get_property_list(List<PropertyInfo> *p_list) const {
	EntityManager *em = EntityManager::get_singleton();
	if (!em || !em->is_entity_valid(entity_id)) {
		return;
	}

	p_list->push_back(PropertyInfo(Variant::INT, "Entity/ID", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY));

	// Group: Transform
	if (em->has_component<TransformComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Transform", PROPERTY_HINT_NONE, "Transform/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform/x"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform/y"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform/z"));
	}

	// Group: Transform2D
	if (em->has_component<Transform2DComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Transform2D", PROPERTY_HINT_NONE, "Transform2D/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform2D/x"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform2D/y"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform2D/rotation"));
	}

	// Group: Debug
	if (em->has_component<DebugComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Debug", PROPERTY_HINT_NONE, "Debug/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::STRING_NAME, "Debug/label"));
	}

	// Group: Audio
	if (em->has_component<AudioComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Audio", PROPERTY_HINT_NONE, "Audio/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Audio/volume"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Audio/pitch"));
		p_list->push_back(PropertyInfo(Variant::BOOL, "Audio/is_3d"));
	}

	// Group: WorldTransform
	if (em->has_component<WorldTransformComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "WorldTransform", PROPERTY_HINT_NONE, "WorldTransform/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "WorldTransform/x", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "WorldTransform/y", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "WorldTransform/z", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY));
	}

	// Group: Animation
	if (em->has_component<AnimationComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Animation", PROPERTY_HINT_NONE, "Animation/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Animation/fps"));
		p_list->push_back(PropertyInfo(Variant::INT, "Animation/current_frame"));
	}
}

ECSEntityProxy::ECSEntityProxy() {}
