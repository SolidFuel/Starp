/****
 * Starp - Stable Random Arpeggiator Plugin 
 * Copyright (C) 2023 Mark Hollomon
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version. This program is distributed in the hope that it 
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file
 * in the root directory.
 ****/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "HashRandom.hpp"
#include "AlgorithmEnum.hpp"
#include "ValueListener.hpp"

#include "Starp.hpp"

struct RandomParameters {
    // Even with the L suffix it uses a 32 bit int. So
    // need to cast to int64 intentionally.
    juce::Value seed_value{juce::var{juce::int64{0L}}};

    void pick_new_key() {
        juce::Random rng{};
        seed_value = rng.nextInt64();
    }
};

struct LinearParameters {
    enum Direction { Up, Down };
    juce::Value direction{Up};
    int get_direction() const { return int(direction.getValue()); }
    juce::Value zigzag{juce::var{false}};
    bool get_zigzag() const { return bool(zigzag.getValue()); }
};

class AlgorithmBase {

public:

    AlgorithmBase() = default;

    virtual int getNextNote(double timeline_slot, const juce::SortedSet<int> &notes, bool notes_changed)  = 0;

    virtual void reset() {
    }

    virtual Algorithm get_algo() const = 0;

    virtual ~AlgorithmBase() {}

};

class LinearAlgorithm : public AlgorithmBase {

private: 
    LinearParameters * p_;

    bool zigzag = false;
    int direction = LinearParameters::Direction::Up;

    ValueListener direction_listener_;
    ValueListener zigzag_listener_;

    void update_parameters() {
        DBGLOG("LinearAlgorithm::update_parameters called")
        zigzag = bool(p_->zigzag.getValue());
        direction = int(p_->direction.getValue());
    }

public :
    LinearAlgorithm(LinearParameters * p) : p_{p} {
        update_parameters();

        direction_listener_.onChange = [this](juce::Value &) {
            update_parameters();
        };

        p_->direction.addListener(&direction_listener_);

        zigzag_listener_.onChange = [this](juce::Value &) {
            update_parameters();
        };

        p_->zigzag.addListener(&zigzag_listener_);

    }

    ~LinearAlgorithm() {
        p_->direction.removeListener(&direction_listener_);
        p_->zigzag.removeListener(&zigzag_listener_);
    }

    Algorithm get_algo() const { return Algorithm::Linear; }

    int getNextNote(double slot, const juce::SortedSet<int> &notes, bool) override {

        auto note_count = notes.size();

        if (note_count == 0) {
            return -1;
        }


        int index = 0;

        if (zigzag) {
            // -2 because we don't want to repeat the top and bottom.
            auto cycle_length = 2 * note_count - 2;

            index = int(slot) % cycle_length;
            index -= (index >= note_count) *(index - note_count +2);

        } else {
            index = int(slot) % note_count;
        }

        if (direction == LinearParameters::Direction::Down) {
            index = note_count - 1 - index;
        }
        return notes[index];
    }
};

class RandomAlgorithm : public AlgorithmBase, juce::Value::Listener {

    // Non owning
    RandomParameters *p_;

    juce::int64 key_;

    juce::SortedSet<int> available_notes;

    int last_note = -1;


public:

    RandomAlgorithm(RandomParameters *p) : p_(p) {
        key_ = juce::int64(p_->seed_value.getValue());
        p_->seed_value.addListener(this);
    }

    void valueChanged(juce::Value &) {
        DBGLOG("RandomAlgorithm::valueChanged called")
        key_ = juce::int64(p_->seed_value.getValue());
    }

    juce::int64 getKey() const {
        return key_;
    }

    void reset() override {
        AlgorithmBase::reset();
        last_note = -1;
        available_notes.clearQuick();
    }

    Algorithm get_algo() const { return Algorithm:: Random; }


    int getNextNote(double timeline_slot, const juce::SortedSet<int> &notes, bool notes_changed) override {

        if (available_notes.isEmpty()) {
            available_notes.addSet(notes);
        } else if (notes_changed) {
            available_notes.clearQuick();
            // Add all the stuff from the new set.
            available_notes.addSet(notes);
        }

        int num_notes = available_notes.size();
        DBGLOG("available note count = ", num_notes)

        if (num_notes > 0) {
            if (num_notes == 1) {
                last_note = available_notes[0];
                available_notes.clear();
                return last_note;
            } else {
                last_note = getRandom(timeline_slot, last_note, available_notes);
                available_notes.removeValue(last_note);
                return last_note;
            }
        } else {
            return -1;
        }
    }

    virtual ~RandomAlgorithm() override {
        // We don't own the p_, so it might out live us.
        // explicitly remove the listener.
        p_->seed_value.removeListener(this);
    };

private :
    int getRandom(double slot, int note_to_avoid, const juce::SortedSet<int> & notes) {


        HashRandom rng{"Note", key_, slot};

        for (int j=0; j < 20; ++j) {
            // need to do better, but this is an okay proof of concept
           int maybe =  notes[rng.nextInt(0, notes.size())];
           if (maybe != note_to_avoid)
            return maybe;
        }

        return -1;
    }

};
