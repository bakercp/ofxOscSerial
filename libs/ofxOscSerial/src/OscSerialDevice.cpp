//
// Copyright (c) 2017 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:    MIT
//


#include "ofx/IO/OscSerialDevice.h"


namespace ofx {
namespace IO {

    
OSCSerialDevice::OSCSerialDevice():
    _bufferListener(packetEvents.onSerialBuffer.newListener(this, &OSCSerialDevice::_onSerialBuffer)),
    _errorListener(packetEvents.onSerialError.newListener(this, &OSCSerialDevice::_onSerialError))
{
}

    
OSCSerialDevice::~OSCSerialDevice()
{
}

    
void OSCSerialDevice::send(const ofxOscMessage& message)
{
    char buffer[SEND_BUFFER_SIZE];

    osc::OutboundPacketStream p(buffer, SEND_BUFFER_SIZE);
    
    bool wrapInBundle = true;
    
    if (wrapInBundle)
    {
        p << osc::BeginBundleImmediate;
    }
    
    appendMessage(message, p);
    
    if (wrapInBundle)
    {
        p << osc::EndBundle;
    }
    
    SLIPPacketSerialDevice::send(ByteBuffer(p.Data(), p.Size()));
}

    
void OSCSerialDevice::appendMessage(const ofxOscMessage& message,
                                    osc::OutboundPacketStream& p)
{
    p << osc::BeginMessage(message.getAddress().data());
    
    for (std::size_t i = 0; i < message.getNumArgs(); ++i)
    {
        switch (message.getArgType(i)) {
            case OFXOSC_TYPE_INT32:
                p << message.getArgAsInt32(i);
                break;
            case OFXOSC_TYPE_INT64:
                p << osc::int64(message.getArgAsInt64(i));
                break;
            case OFXOSC_TYPE_FLOAT:
                p << message.getArgAsFloat(i);
                break;
            case OFXOSC_TYPE_DOUBLE:
                p << message.getArgAsDouble(i);
                break;
            case OFXOSC_TYPE_STRING:
                p << message.getArgAsString(i).data();
            case OFXOSC_TYPE_SYMBOL:
                p << osc::Symbol(message.getArgAsString(i).data());
                break;
            case OFXOSC_TYPE_CHAR:
                p << message.getArgAsChar(i);
                break;
            case OFXOSC_TYPE_MIDI_MESSAGE:
                p << osc::MidiMessage(message.getArgAsMidiMessage(i));
                break;
            case OFXOSC_TYPE_TRUE:
            case OFXOSC_TYPE_FALSE:
                p << message.getArgAsBool(i);
                break;
            case OFXOSC_TYPE_NONE:
                p << osc::NilType();
                break;
            case OFXOSC_TYPE_TRIGGER:
                p << osc::InfinitumType();
                break;
            case OFXOSC_TYPE_TIMETAG:
                p << osc::TimeTag(message.getArgAsTimetag(i));
                break;
            case OFXOSC_TYPE_BLOB:
            {
                ofBuffer buff = message.getArgAsBlob(i);
                p << osc::Blob(buff.getData(), osc::osc_bundle_element_size_t(buff.size()));
                break;
            }
//            case OFXOSC_TYPE_BUNDLE:
//                // call recursively?
//                
//                break;
            case OFXOSC_TYPE_RGBA_COLOR:
                p << osc::RgbaColor(message.getArgAsRgbaColor(i));
                break;
            default:
                ofLogError("OSCSerialDevice::appendMessage") << "appendMessage(): bad argument type " << message.getArgType( i );
                assert(false);
                break;
        }
    }
    
    p << osc::EndMessage;

}
    
    
void OSCSerialDevice::_onSerialBuffer(const SerialBufferEventArgs& args)
{
    osc::ReceivedPacket p(args.buffer().getData(), args.buffer().size());
    
    if (p.IsBundle())
        processReceivedBundle(osc::ReceivedBundle(p));
    else
        processReceivedMessage(osc::ReceivedMessage(p));
}


void OSCSerialDevice::_onSerialError(const SerialBufferErrorEventArgs& args)
{
    ofNotifyEvent(onSerialError, args, this);
}


void OSCSerialDevice::processReceivedBundle(const osc::ReceivedBundle& b)
{
    for (auto i = b.ElementsBegin(); i != b.ElementsEnd(); ++i)
    {
        if (i->IsBundle())
            processReceivedBundle(osc::ReceivedBundle(*i));
        else
            processReceivedMessage(osc::ReceivedMessage(*i));
    }
}


void OSCSerialDevice::processReceivedMessage(const osc::ReceivedMessage& m)
{
    ofxOscMessage msg;
    msg.setAddress(m.AddressPattern());
    msg.setRemoteEndpoint(port(), 0);
    
    for (auto arg = m.ArgumentsBegin(); arg != m.ArgumentsEnd(); ++arg)
    {
        if(arg->IsInt32())
            msg.addIntArg(arg->AsInt32Unchecked());
        else if(arg->IsInt64())
            msg.addInt64Arg(arg->AsInt64Unchecked());
        else if( arg->IsFloat())
            msg.addFloatArg(arg->AsFloatUnchecked());
        else if(arg->IsDouble())
            msg.addDoubleArg(arg->AsDoubleUnchecked());
        else if(arg->IsString())
            msg.addStringArg(arg->AsStringUnchecked());
        else if(arg->IsSymbol())
            msg.addSymbolArg(arg->AsSymbolUnchecked());
        else if(arg->IsChar())
            msg.addCharArg(arg->AsCharUnchecked());
        else if(arg->IsMidiMessage())
            msg.addMidiMessageArg(arg->AsMidiMessageUnchecked());
        else if(arg->IsBool())
            msg.addBoolArg(arg->AsBoolUnchecked());
        else if(arg->IsNil())
            msg.addNoneArg();
        else if(arg->IsInfinitum())
            msg.addTriggerArg();
        else if(arg->IsTimeTag())
            msg.addTimetagArg(arg->AsTimeTagUnchecked());
        else if(arg->IsRgbaColor())
            msg.addRgbaColorArg(arg->AsRgbaColorUnchecked());
        else if(arg->IsBlob())
        {
            const char * dataPtr = nullptr;
            osc::osc_bundle_element_size_t size = 0;
            arg->AsBlobUnchecked((const void*&)dataPtr, size);
            ofBuffer buffer(dataPtr, size);
            msg.addBlobArg(buffer);
        }
        else
        {
            ofLogError("OSCSerialDevice::processReceivedMessage") << "Argument in message: "
            << m.AddressPattern() << " is an unknown type "
            << (int) arg->TypeTag() << " '" << (char) arg->TypeTag() << "'";
            break;
        }
    }
    
    ofNotifyEvent(onOscMessage, msg, this);
}

    
    
} } // namespace ofx::IO
