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

#include "../Algorithm.hpp"
#include "../ValueListener.hpp"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>


using  SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

class LinearAlgoOptionsComponent : public juce::Component {
public :
    LinearAlgoOptionsComponent(LinearParameters * parms);

    void paint(juce::Graphics&) override;
    void resized() override;

    ~LinearAlgoOptionsComponent();


private :

    LinearParameters *params_;

    juce::TextButton up_button_;
    juce::TextButton down_button_;

    juce::ToggleButton zigzag_button_;

    ValueListener direction_listener_;
    ValueListener zigzag_listener_;

    void update_direction();
    void update_zigzag();




};