//
// Copyright (c) 2017 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:    MIT
//


#pragma once


#include "ofx/IO/PacketSerialDevice.h"
#include "ofxOsc.h"


namespace ofx {
namespace IO {


class OSCSerialDevice: protected SLIPPacketSerialDevice
{
public:
    OSCSerialDevice();
    virtual ~OSCSerialDevice();

    using SLIPPacketSerialDevice::setup;

    void send(const ofxOscMessage& message);

    ofEvent<const ofxOscMessage> onOscMessage;
    ofEvent<const SerialBufferErrorEventArgs> onSerialError;

    /// \brief Register a class to receive notifications for all events.
    /// \param listener a pointer to the listener class.
    /// \param order the event order.
    /// \tparam ListenerClass The listener class type.
    template<class ListenerClass>
    void registerAllEvents(ListenerClass* listener, int order = OF_EVENT_ORDER_AFTER_APP)
    {
        ofAddListener(onOscMessage, listener, &ListenerClass::onOscMessage, order);
        ofAddListener(onSerialError, listener, &ListenerClass::onSerialError, order);
    }
    
    /// \brief Unregister a class to receive notifications for all events.
    /// \param listener a pointer to the listener class.
    /// \param order the event order.
    /// \tparam ListenerClass The listener class type.
    template<class ListenerClass>
    void unregisterAllEvents(ListenerClass* listener, int order = OF_EVENT_ORDER_AFTER_APP)
    {
        ofRemoveListener(onOscMessage, listener, &ListenerClass::onOscMessage, order);
        ofRemoveListener(onSerialError, listener, &ListenerClass::onSerialError, order);
    }

    enum
    {
        SEND_BUFFER_SIZE = 8192
    };

private:
    void _onSerialBuffer(const SerialBufferEventArgs& args);
    void _onSerialError(const SerialBufferErrorEventArgs& args);

    void appendMessage(const ofxOscMessage& message,
                       osc::OutboundPacketStream& p);

    void processReceivedBundle(const osc::ReceivedBundle& b);
    void processReceivedMessage(const osc::ReceivedMessage& m);

    ofEventListener _bufferListener;
    ofEventListener _errorListener;
};


} } // namespace ofx::IO
