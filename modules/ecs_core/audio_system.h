#ifndef ECS_AUDIO_SYSTEM_H
#define ECS_AUDIO_SYSTEM_H

#include "core/object/object.h"
#include "core/templates/rid.h"
#include "core/templates/vector.h"
#include "core/typedefs.h"

// Native ECS Audio System.
// Directly pushes spatial data to AudioServer, bypassing heavy AudioStreamPlayer3D nodes.
class AudioSystem : public Object {
    GDCLASS(AudioSystem, Object);

private:
    static AudioSystem *singleton;
    
    // Internal mapping of entity to AudioServer voices
    Vector<RID> active_voices;

protected:
    static void _bind_methods();

public:
    static AudioSystem *get_singleton();

    void play_spatial_sound(uint64_t p_entity, RID p_stream);
    void process_audio_updates();
    void _on_audio_component_removed(uint64_t p_entity);

    AudioSystem();
    ~AudioSystem();
};

#endif // ECS_AUDIO_SYSTEM_H
