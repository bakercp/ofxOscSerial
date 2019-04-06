//
// Copyright (c) 2017 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:    MIT
//

#pragma once


#include "ofMain.h"
#include "ofxOscSerial.h"


class ofApp: public ofBaseApp
{
public:
    void setup() override;
    void draw() override;

    void keyPressed(int key) override;

    void onOscMessage(const ofxOscMessage& message);
    void onSerialError(const ofxIO::SerialBufferErrorEventArgs& error);

    bool ledState = false;

    ofxIO::OSCSerialDevice device;

};
