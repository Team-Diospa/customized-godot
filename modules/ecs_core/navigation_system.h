/**************************************************************************/
/*  navigation_system.h                                                   */
/**************************************************************************/

#pragma once

#include "core/object/object.h"
#include "core/typedefs.h"

class NavigationSystem : public Object {
	GDCLASS(NavigationSystem, Object);

private:
	static NavigationSystem *singleton;

protected:
	static void _bind_methods();

public:
	static NavigationSystem *get_singleton();

	void process_navigation_updates(float p_delta);

	NavigationSystem();
	~NavigationSystem();
};
