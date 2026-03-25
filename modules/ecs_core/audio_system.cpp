#include "audio_system.h"
#include "entity_manager.h"
#include "servers/audio/audio_server.h"

AudioSystem *AudioSystem::singleton = nullptr;

AudioSystem *AudioSystem::get_singleton() { return singleton; }

void AudioSystem::_bind_methods() {}

AudioSystem::AudioSystem() {
    singleton = this;
    active_voices.resize(1000); // Pre-allocate voices for performance

    EntityManager *em = EntityManager::get_singleton();
    if (em) {
        SparseSet<AudioComponent>* audios = em->get_audios();
        if (audios) {
            audios->register_on_removed(callable_mp(this, &AudioSystem::_on_audio_component_removed));
        }
    }
}

void AudioSystem::_on_audio_component_removed(uint64_t p_entity) {
    EntityManager *em = EntityManager::get_singleton();
    if (!em) return;

    SparseSet<AudioComponent>* audios = em->get_audios();
    if (audios && audios->has(p_entity)) {
        AudioComponent& ac = audios->get(p_entity);
        if (ac.stream_rid.is_valid()) {
            AudioServer::get_singleton()->free_rid(ac.stream_rid);
        }
    }
}

AudioSystem::~AudioSystem() {
    if (singleton == this) singleton = nullptr;

    EntityManager *em = EntityManager::get_singleton();
    if (em) {
        SparseSet<AudioComponent>* audios = em->get_audios();
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
    if (!em || !as) return;

    SparseSet<AudioComponent>* audios = em->get_audios();
    SparseSet<TransformComponent>* transforms = em->get_transforms();
    if (!audios || !transforms) return;

    const Vector<uint64_t>& entities = audios->get_dense_raw();
    for (int i = 0; i < entities.size(); i++) {
        uint64_t entity = entities[i];
        AudioComponent& ac = audios->get(entity);
        
        if (!ac.stream_rid.is_valid()) continue;

        if (ac.is_3d && transforms->has(entity)) {
            TransformComponent& t = transforms->get(entity);
            // Update 3D position logic here
        }
    }
}
