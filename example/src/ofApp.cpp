//
// Copyright (c) 2017 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:    MIT
//


#include "ofApp.h"


void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);

    ofEnableAlphaBlending();

    auto devicesInfo = ofxIO::SerialDeviceUtils::listDevices();

    ofLogNotice("ofApp::setup") << "Connected Devices: ";

    for (auto& device: devicesInfo) ofLogNotice("ofApp::setup") << "\t" << device;

    if (!devicesInfo.empty())
    {
        // Connect to the first matching device.
        bool success = device.setup(devicesInfo[0], 115200);

        if (success)
        {
            device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devicesInfo[0];
        }
        else ofLogNotice("ofApp::setup") << "Unable to setup " << devicesInfo[0];
    }
    else ofLogNotice("ofApp::setup") << "No devices connected.";
}


void ofApp::draw()
{
    if (ledState)
        ofBackground(ofColor::yellow);
    else
        ofBackground(ofColor::black);

    ofDrawBitmapStringHighlight("Press a key to toggle the LED.", 20, 20);
}


void ofApp::keyPressed(int key)
{
    ledState = !ledState;

    ofxOscMessage m;
    m.setAddress("/led");
    m.addInt32Arg(ledState);

    device.send(m);
}


void ofApp::onOscMessage(const ofxOscMessage& message)
{
    ofLog() << "Got OSC Message: " << message;
}

void ofApp::onSerialError(const ofxIO::SerialBufferErrorEventArgs& error)
{
    ofLogError() << "Got OSC Error: " << error.exception().displayText();
}
