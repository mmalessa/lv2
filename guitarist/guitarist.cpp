
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <new>
#include <array>
#include <utility>
#include "lv2.h"
#include <lv2/atom/atom.h>
#include <lv2/urid/urid.h>
#include <lv2/midi/midi.h>
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include <lv2/core/lv2_util.h>
#include <lv2/atom/util.h>

enum PortGroups
{
    PORT_MIDI_IN    = 0,
    PORT_MIDI_OUT   = 1,
    PORT_NR         = 2
};

struct Urids
{
    LV2_URID midi_MidiEvent;
    LV2_URID atom_Object;
    LV2_URID atom_Blank;

    LV2_URID time_Position;
    LV2_URID time_barBeat;
    LV2_URID time_beatsPerMinute;
    LV2_URID time_speed;

    LV2_URID time_bar;
    LV2_URID time_beatsPerBar;
    LV2_URID time_beat;
};

class Guitarist 
{
private:
    const LV2_Atom_Sequence* midi_in_ptr ;
    LV2_Atom_Sequence* midi_out_ptr ;
    LV2_URID_Map* map ;
    Urids urids;
    double rate;
public:
    Guitarist (const double sample_rate, const LV2_Feature *const *features);
    void connectPort (const uint32_t port, void* data_location);
    void run (const uint32_t sample_count);
};

Guitarist::Guitarist (const double sample_rate, const LV2_Feature *const *features) :
    midi_in_ptr (nullptr),
    midi_out_ptr (nullptr),
    map (nullptr),
    rate (sample_rate)
{

    const char* missing = lv2_features_query
    (
        features,
        LV2_URID__map, &map, true,
        NULL
    );

    if (missing) throw std::invalid_argument ("Feature map not provided by the host. Can't instantiate mySimpleSynth");

    urids.midi_MidiEvent = map->map (map->handle, LV2_MIDI__MidiEvent);
    urids.atom_Object = map->map (map->handle, LV2_ATOM__Object);
    urids.atom_Blank = map->map (map->handle, LV2_ATOM__Blank);
    urids.time_Position = map->map (map->handle, LV2_TIME__Position);
    urids.time_barBeat = map->map (map->handle, LV2_TIME__barBeat);
    urids.time_beatsPerMinute = map->map (map->handle, LV2_TIME__beatsPerMinute);
    urids.time_speed = map->map (map->handle, LV2_TIME__speed);

    urids.time_bar = map->map (map->handle, LV2_TIME__bar);
    urids.time_beatsPerBar = map->map (map->handle, LV2_TIME__beatsPerBar);
    urids.time_beat = map->map (map->handle, LV2_TIME__beat);
}
    


void Guitarist::connectPort (const uint32_t port, void* data_location)
{
    switch (port)
    {
    case PORT_MIDI_IN:
        // midi_in_ptr = static_cast<const LV2_Atom_Sequence*> (data_location);
        midi_in_ptr = (const LV2_Atom_Sequence*)data_location;
        break;

    case PORT_MIDI_OUT:
        // midi_out_ptr = static_cast<const LV2_Atom_Sequence*> (data_location);
        midi_out_ptr = (LV2_Atom_Sequence*)data_location;
        break;
    
    default:
        break;
    }
}

void Guitarist::run (const uint32_t sample_count)
{
    /* check if all ports connected */
    if ((!midi_out_ptr) || (!midi_in_ptr)) return;

    // Get the capacity
    const uint32_t out_capacity = midi_out_ptr->atom.size;

    // Write an empty Sequence header to the output
    lv2_atom_sequence_clear(midi_out_ptr);
    midi_out_ptr->atom.type = midi_in_ptr->atom.type;

    /* analyze incomming MIDI data */
    uint32_t last_frame = 0;
    
    
    for (const LV2_Atom_Event* ev = lv2_atom_sequence_begin(&midi_in_ptr->body);
       !lv2_atom_sequence_is_end(&midi_in_ptr->body, midi_in_ptr->atom.size, ev);
       ev = lv2_atom_sequence_next(ev))
    // LV2_ATOM_SEQUENCE_FOREACH (midi_in_ptr, ev)
    {
        lv2_atom_sequence_append_event(midi_out_ptr, out_capacity, ev);
			
        if (ev->body.type == urids.atom_Blank || ev->body.type == urids.atom_Object) {
            lv2_atom_sequence_append_event(midi_out_ptr, out_capacity, ev);
			const LV2_Atom_Object* obj = (LV2_Atom_Object*)&ev->body;
			if (obj->body.otype == urids.time_Position) {

                // Received new transport position/speed
                LV2_Atom* barBeat  = NULL;
                LV2_Atom* bpm   = NULL;
                LV2_Atom* speed = NULL;
                LV2_Atom* bar = NULL;
                // LV2_Atom* beat = NULL;
                // LV2_Atom* beatsPerBar = NULL;

                // clang-format off
                lv2_atom_object_get(obj,
                      urids.time_barBeat, &barBeat,
                      urids.time_beatsPerMinute, &bpm,
                      urids.time_speed, &speed,
                      urids.time_bar, &bar,
                    //   urids.time_beatsPerBar, &beatsPerBar,
                    //   urids.time_beat, &beat,
                      NULL);
                // clang-format on
            
                // BPM ((LV2_Atom_Float*)bpm)->body

                std::cerr << "time_bpm: " << std::to_string(((LV2_Atom_Float*)bpm)->body) << std::endl;
                // std::cerr << "time_beatsPerBar: " << std::to_string(((LV2_Atom_Float*)bar)->body) << std::endl;
                std::cerr << "time_bar: " << std::to_string(((LV2_Atom_Long*)bar)->body) << std::endl;
                // std::cerr << "time_beat" << std::to_string(((LV2_Atom_Double*)beat)->body) << std::endl;
				std::cerr << "time_barBeat: " << std::to_string(((LV2_Atom_Float*)barBeat)->body) << std::endl;
                std::cerr << std::endl;

			}
		} else if (ev->body.type == urids.midi_MidiEvent)
        {
            const uint8_t* const msg = reinterpret_cast<const uint8_t*> (ev + 1);
            const uint8_t typ = lv2_midi_message_type (msg);

            // lv2_atom_sequence_append_event(midi_out_ptr, out_capacity, ev);
            switch (typ)
            {
            case LV2_MIDI_MSG_NOTE_ON:
                std::cerr << "Note " << std::to_string(msg[1]) << " ON (vel: " << std::to_string(msg[2]) << ")" << std::endl;
                // Forward note to output
                // lv2_atom_sequence_append_event(midi_out_ptr, out_capacity, ev);

                // key.press
                // (
                //     static_cast<Waveform> (control[CONTROL_WAVEFORM]),
                //     msg[1] /* note */,
                //     msg[2] /* velocity */,
                //     {
                //         control[CONTROL_ATTACK],
                //         control[CONTROL_DECAY],
                //         control[CONTROL_SUSTAIN],
                //         control[CONTROL_RELEASE]
                //     }
                // );
                break;

            case LV2_MIDI_MSG_NOTE_OFF:
                std::cerr << "Note " << std::to_string(msg[1]) << " OFF (vel: " << std::to_string(msg[2]) << ")" << std::endl;
                // lv2_atom_sequence_append_event(midi_out_ptr, out_capacity, ev);
                // key.release (msg[1], msg[2]);
                break;

            // case LV2_MIDI_MSG_CONTROLLER:
            //     lv2_atom_sequence_append_event(midi_out_ptr, out_capacity, ev);
            //     std::cerr << "Controller " << std::to_string(msg[1]) << ", " << std::to_string(msg[2]) << std::endl;
            //     // if (msg[1] == LV2_MIDI_CTL_ALL_NOTES_OFF) key.release();
            //     // else if (msg[1] == LV2_MIDI_CTL_ALL_SOUNDS_OFF) key.mute();
            //     break;
            
            default:
                // lv2_atom_sequence_append_event(midi_out_ptr, out_capacity, ev);
                std::cerr << "Default " << std::to_string(typ) << ", " << std::to_string(msg[1]) << ", " << std::to_string(msg[2]) << std::endl;
                break;
            }
        
        } else {
            std::cerr << "BodyType: " << std::to_string(ev->body.type) << std::endl;
        }
        
        
    }
}

/* internal core methods */
static LV2_Handle instantiate (const struct LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
    Guitarist* g = nullptr;
    try 
    {
        g = new Guitarist (sample_rate, features);
    } 
    catch (const std::invalid_argument& ia) 
    {
        std::cerr << ia.what() << std::endl;
        return nullptr;
    }
    catch (const std::bad_alloc& ba)
    {
        std::cerr << "Failed to allocate memory. Can't instantiate Guitarist" << std::endl;
        return nullptr;
    }
    return g;
}

static void connect_port (LV2_Handle instance, uint32_t port, void *data_location)
{
    Guitarist* g = static_cast <Guitarist*> (instance);
    if (g) g->connectPort (port, data_location);
}

static void activate (LV2_Handle instance)
{
    /* not needed here */
}

static void run (LV2_Handle instance, uint32_t sample_count)
{
    Guitarist* g = static_cast <Guitarist*> (instance);
    if (g) g->run (sample_count);
}

static void deactivate (LV2_Handle instance)
{
    /* not needed here */
}

static void cleanup (LV2_Handle instance)
{
    Guitarist* g = static_cast <Guitarist*> (instance);
    if (g) delete g;
}

static const void* extension_data (const char *uri)
{
    return NULL;
}

/* descriptor */
static LV2_Descriptor const descriptor =
{
    "https://github.com/mmalessa/lv2/guitarist",
    instantiate,
    connect_port,
    activate,
    run,
    deactivate /* or NULL */,
    cleanup,
    extension_data /* or NULL */
};

/* interface */
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor (uint32_t index)
{
    if (index == 0) return &descriptor;
    else return NULL;
}