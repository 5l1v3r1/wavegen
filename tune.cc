#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

/**
 * The exception that strtonum throws to indicate an error
 */
class StrToNumError : public std::runtime_error {
public:
    StrToNumError() : std::runtime_error("Invalid number format") {}
};

/**
 * A wrapper to C-style strtol function family
 * Usage:
 *   long result1 = strtonum(std::strtol, "42", 10);
 *   double result2 = strtonum(std::strtod, "0.618");
 * Throws:
 *   StrToNumError, when input is not a valid number
 */
template <typename Fn, typename ...Args>
static inline auto strtonum(Fn fn, const char *str, Args &&...args) -> decltype(fn(str, nullptr, std::forward<Args>(args)...)) {
    if(str[0] != '\0') {
        char *endptr;
        auto result = fn(str, &endptr, std::forward<Args>(args)...);
        if(endptr == &str[std::strlen(str)]) {
            return result;
        } else {
            throw StrToNumError();
        }
    } else {
        throw StrToNumError();
    }
}

/**
 * Conversion among note name, MIDI key ID, and frequency
 */
class Tuner {
public:
    static uint8_t note_name_to_midi_id(const std::string &note_name);
    static double midi_id_to_freq(uint8_t midi_id);
    static double midi_id_to_freq(double midi_id);
    static double note_name_to_freq(const std::string &note_name) {
        return midi_id_to_freq(note_name_to_midi_id(note_name));
    }
    class TunerError;
};

class Tuner::TunerError : public std::runtime_error {
public:
    TunerError() : std::runtime_error("Invalid note name") {}
};

uint8_t Tuner::note_name_to_midi_id(const std::string &note_name) {
    static const int note_name_table[7] = {
      /* A   B  C  D  E  F  G */
         9, 11, 0, 2, 4, 5, 7
    };

    if(note_name.length() < 2)
        throw TunerError();
    char bare_name = note_name[0] & 0xdf;
    if(bare_name < 'A' || bare_name > 'G')
        throw TunerError();
    int midi_id = note_name_table[bare_name-'A'];

    const char *sharp_ptr = &note_name.c_str()[1];
    if(*sharp_ptr == '#') {
        ++midi_id;
        ++sharp_ptr;
    }

    long octave_id;
    try {
        octave_id = strtonum(std::strtol, sharp_ptr, 10);
    } catch(StrToNumError) {
        throw TunerError();
    }
    if(octave_id < -1 || octave_id > 9)
        throw TunerError();

    midi_id += (int(octave_id)+1)*12;
    if(midi_id >= 128)
        throw TunerError();
    return uint8_t(midi_id);
}

double Tuner::midi_id_to_freq(uint8_t midi_id) {
    // https://en.wikipedia.org/wiki/MIDI_Tuning_Standard
    static const double M_2_POW_INV_12 = 1.05946309435929526456; // 2^(1/12)
    return std::pow(M_2_POW_INV_12, int(midi_id)-69) * 440;
}

double Tuner::midi_id_to_freq(double midi_id) {
    static const double M_2_POW_INV_12 = 1.05946309435929526456; // 2^(1/12)
    return std::pow(M_2_POW_INV_12, midi_id-69) * 440;
}

int main(void) {
    Tuner tuner;
    std::string note_name;
    for(;;) {
        std::cerr << "Note name: ";
        if(!std::getline(std::cin, note_name)) {
            break;
        }
        auto midi_id = tuner.note_name_to_midi_id(note_name);
        auto freq = tuner.midi_id_to_freq(midi_id);
        std::cout << "ID: 0x" << std::hex << unsigned(midi_id) << ", Freq: " << freq << " Hz" << std::endl;
    }
    return 0;
}
