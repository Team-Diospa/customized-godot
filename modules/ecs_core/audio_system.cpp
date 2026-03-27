/**************************************************************************/
/*  audio_system.cpp                                                      */
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

#include "audio_system.h"

#include "entity_manager.h"

#include "core/object/callable_mp.h"
#include "core/templates/vector.h"
#include "servers/audio/audio_server.h"

AudioSystem *AudioSystem::singleton = nullptr;

AudioSystem *AudioSystem::get_singleton() {
	return singleton;
}

void AudioSystem::_bind_methods() {}

AudioSystem::AudioSystem() {
	singleton = this;

	EntityManager *em = EntityManager::get_singleton();
	if (em) {
		SparseSet<AudioComponent> *audios = em->get_audios();
		if (audios) {
			audios->register_on_removed(callable_mp(this, &AudioSystem::_on_audio_component_removed));
		}
	}
}

void AudioSystem::_on_audio_component_removed(uint64_t p_entity) {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	SparseSet<AudioComponent> *audios = em->get_audios();
	if (audios && audios->has(p_entity)) {
		AudioComponent &ac = audios->get(p_entity);
		if (ac.stream_rid.is_valid()) {
			// AudioServer::get_singleton()->free_rid(ac.stream_rid); // Godot 4 AudioServer doesn't have free_rid. 
			// In production, playback is managed via AudioStreamPlayback or SamplePlayback.
		}
	}
}

AudioSystem::~AudioSystem() {
	if (singleton == this) {
		singleton = nullptr;
	}

	EntityManager *em = EntityManager::get_singleton();
	if (em) {
		SparseSet<AudioComponent> *audios = em->get_audios();
		if (audios) {
			// Unregister to avoid calling deleted system during shutdown
			audios->unregister_on_removed(callable_mp(this, &AudioSystem::_on_audio_component_removed));
		}
	}
}

void AudioSystem::play_spatial_sound(uint64_t p_entity, RID p_stream) {
	AudioServer *as = AudioServer::get_singleton();
	// Implementation would use AudioServer::get_singleton()->...
}

void AudioSystem::process_audio_updates() {
	EntityManager *em = EntityManager::get_singleton();
	if (!em) {
		return;
	}

	SparseSet<AudioVoiceComponent> *voices = em->get_audio_voices();
	SparseSet<WorldTransformComponent> *worlds = em->get_world_transforms();
	if (!voices || !worlds) {
		return;
	}

	AudioServer *as = AudioServer::get_singleton();
	const Vector<uint64_t> &entities = voices->get_dense_raw();

	// In a real production scenario, we'd get the listener position
	Vector3 listener_pos = Vector3(0, 0, 0); 
	float culling_dist_sq = 2500.0f; // 50 units

	for (int i = 0; i < entities.size(); i++) {
		uint64_t entity = entities[i];
		AudioVoiceComponent &vc = voices->get(entity);

		if (!worlds->has(entity)) {
			continue;
		}

		const WorldTransformComponent &wt = worlds->get(entity);
		Vector3 pos(wt.x, wt.y, wt.z);
		float dist_sq = pos.distance_squared_to(listener_pos);

		if (dist_sq > culling_dist_sq) {
			if (vc.is_active) {
				// Culling logic: if too far, we might want to pause or stop
				vc.is_active = false;
				// TODO: as->voice_stop(vc.stream_instance);
			}
			continue;
		}

		vc.is_active = true;
		// Sync with AudioServer
		if (vc.stream_instance.is_valid()) {
			// rs->skeleton_bone_set_transform pattern for audio
			// TODO: as->voice_set_position(vc.stream_instance, pos);
			// TODO: as->voice_set_volume(vc.stream_instance, vc.volume);
		}
	}
}
