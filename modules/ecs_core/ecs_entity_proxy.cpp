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
	if (name.begins_with("Transform/")) {
		if (em->has_component<TransformComponent>(entity_id)) {
			TransformComponent &t = em->get_component<TransformComponent>(entity_id);
			if (name == "Transform/x") {
				t.x = p_value;
			} else if (name == "Transform/y") {
				t.y = p_value;
			} else if (name == "Transform/z") {
				t.z = p_value;
			}
			return true;
		}
	} else if (name.begins_with("Debug/")) {
		if (em->has_component<DebugComponent>(entity_id)) {
			DebugComponent &d = em->get_component<DebugComponent>(entity_id);
			if (name == "Debug/label") {
				d.label = p_value;
			}
			return true;
		}
	} else if (name.begins_with("Transform2D/")) {
		if (em->has_component<Transform2DComponent>(entity_id)) {
			Transform2DComponent &t = em->get_component<Transform2DComponent>(entity_id);
			if (name == "Transform2D/x") {
				t.x = p_value;
			} else if (name == "Transform2D/y") {
				t.y = p_value;
			} else if (name == "Transform2D/rotation") {
				t.rotation = p_value;
			}
			return true;
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
	if (name.begins_with("Transform/")) {
		if (!em->has_component<TransformComponent>(entity_id)) {
			return false;
		}
		TransformComponent &t = em->get_component<TransformComponent>(entity_id);
		if (name == "Transform/x") {
			r_ret = t.x;
		} else if (name == "Transform/y") {
			r_ret = t.y;
		} else if (name == "Transform/z") {
			r_ret = t.z;
		}
		return true;
	} else if (name.begins_with("Debug/")) {
		if (!em->has_component<DebugComponent>(entity_id)) {
			return false;
		}
		DebugComponent &d = em->get_component<DebugComponent>(entity_id);
		if (name == "Debug/label") {
			r_ret = d.label;
		}
		return true;
	} else if (name.begins_with("Transform2D/")) {
		if (!em->has_component<Transform2DComponent>(entity_id)) {
			return false;
		}
		Transform2DComponent &t = em->get_component<Transform2DComponent>(entity_id);
		if (name == "Transform2D/x") {
			r_ret = t.x;
		} else if (name == "Transform2D/y") {
			r_ret = t.y;
		} else if (name == "Transform2D/rotation") {
			r_ret = t.rotation;
		}
		return true;
	} else if (name == "Entity/ID") {
		r_ret = (Variant)entity_id;
		return true;
	}

	return false;
}

void ECSEntityProxy::_get_property_list(List<PropertyInfo> *p_list) const {
	EntityManager *em = EntityManager::get_singleton();
	if (!em || !em->is_entity_valid(entity_id)) {
		return;
	}

	p_list->push_back(PropertyInfo(Variant::INT, "Entity/ID", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY));

	if (em->has_component<DebugComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Debug", PROPERTY_HINT_NONE, "Debug/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::STRING_NAME, "Debug/label"));
	}

	if (em->has_component<TransformComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Transform", PROPERTY_HINT_NONE, "Transform/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform/x"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform/y"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform/z"));
	}

	if (em->has_component<Transform2DComponent>(entity_id)) {
		p_list->push_back(PropertyInfo(Variant::NIL, "Transform2D", PROPERTY_HINT_NONE, "Transform2D/", PROPERTY_USAGE_GROUP));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform2D/x"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform2D/y"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "Transform2D/rotation"));
	}
}

void ECSEntityProxy::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_entity", "id"), &ECSEntityProxy::set_entity);
	ClassDB::bind_method(D_METHOD("get_entity"), &ECSEntityProxy::get_entity);
}

ECSEntityProxy::ECSEntityProxy() {}
