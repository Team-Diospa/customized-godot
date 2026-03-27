/**************************************************************************/
/*  animation_system.cpp                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "animation_system.h"
#include "entity_manager.h"

#include "core/object/class_db.h"
#include "core/templates/vector.h"
#include "servers/rendering/rendering_server.h"

AnimationSystem *AnimationSystem::singleton = nullptr;

AnimationSystem *AnimationSystem::get_singleton() {
	return singleton;
}

void AnimationSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("process_animation_updates", "delta"), &AnimationSystem::process_animation_updates);
	ClassDB::bind_method(D_METHOD("process_skeletal_updates"), &AnimationSystem::process_skeletal_updates);
}

AnimationSystem::AnimationSystem() {
	singleton = this;
}

AnimationSystem::~AnimationSystem() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void AnimationSystem::process_animation_updates(float p_delta) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	SparseSet<AnimationComponent> *animations = em->get_animations();
	if (!animations) {
		return;
	}

	const Vector<uint64_t> &entities = animations->get_dense_raw();
	for (int i = 0; i < entities.size(); i++) {
		AnimationComponent &anim = animations->get(entities[i]);

		anim.time_accumulator += p_delta;
		float frame_time = 1.0f / anim.fps;

		if (anim.time_accumulator >= frame_time) {
			anim.current_frame = (anim.current_frame + 1) % anim.total_frames;
			anim.time_accumulator -= frame_time;

			// Simple sprite-sheet UV calculation (uniform row)
			// Assuming the shader expects 0.0-1.0 offsets
			anim.uv_offset_x = (float)anim.current_frame / (float)anim.total_frames;
			anim.uv_offset_y = 0.0f;
		}
	}
}


void AnimationSystem::process_skeletal_updates() {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	SparseSet<ECSSkeletonBridgeComponent> *skeletons = em->get_skeleton_bridges();
	SparseSet<BoneBufferComponent> *bones = em->get_bone_buffers();
	if (!skeletons || !bones) {
		return;
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	const Vector<uint64_t> &entities = skeletons->get_dense_raw();

	for (int i = 0; i < entities.size(); i++) {
		uint64_t entity = entities[i];
		if (!bones->has(entity)) {
			continue;
		}

		const ECSSkeletonBridgeComponent &skel = skeletons->get(entity);
		const BoneBufferComponent &bb = bones->get(entity);

		if (!skel.skeleton.is_valid()) {
			continue;
		}

		// Push each bone transform to the RenderingServer
		// We expect bb.transforms to be in the layout the RS expects
		for (int b = 0; b < skel.bone_count; b++) {
			if (b < bb.transforms.size()) {
				rs->skeleton_bone_set_transform(skel.skeleton, b, bb.transforms[b]);
			}
		}
	}
}
