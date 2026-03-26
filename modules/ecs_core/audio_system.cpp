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
#include "core/templates/vector.h"
#include "core/object/callable_mp.h"
#include "servers/audio/audio_server.h"

AudioSystem *AudioSystem::singleton = nullptr;

AudioSystem *AudioSystem::get_singleton() {
	return singleton;
}

void AudioSystem::_bind_methods() {}

AudioSystem::AudioSystem() {
	singleton = this;
	active_voices.resize(1000); // Pre-allocate voices for performance

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
			// AudioServer::get_singleton()->free_rid(ac.stream_rid); // TODO: Verify Ridley-style audio RID management
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
	AudioServer *as = AudioServer::get_singleton();
	if (!em || !as) {
		return;
	}

	SparseSet<AudioComponent> *audios = em->get_audios();
	SparseSet<TransformComponent> *transforms = em->get_transforms();
	if (!audios || !transforms) {
		return;
	}

	const Vector<uint64_t> &entities = audios->get_dense_raw();
	for (int i = 0; i < entities.size(); i++) {
		uint64_t entity = entities[i];
		AudioComponent &ac = audios->get(entity);

		if (transforms->has(entity)) {
			// Update spatial parameters based on TransformComponent
			// TransformComponent &tc = transforms->get(entity);
			// Example: as->audio_server_set_listener_2d_orientation(0.0f);
		}
	}
}
