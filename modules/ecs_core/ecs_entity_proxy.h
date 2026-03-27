#include "core/object/object.h"

class EntityManager;

class ECSEntityProxy : public Object {
	GDCLASS(ECSEntityProxy, Object);

	uint64_t entity_id = 0;

protected:
	static void _bind_methods();
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	void set_entity(uint64_t p_id);
	uint64_t get_entity() const;
	void sync_telemetry();

	ECSEntityProxy();
};
