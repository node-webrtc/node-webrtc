/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"
#include "talk/app/webrtc/test/fakeconstraints.h"
#include "talk/app/webrtc/peerconnectioninterface.h"

#include "common.h"
#include "peerconnection.h"
#include "datachannel.h"
#include "mediastream.h"

using namespace node;
using namespace v8;

Persistent<Function> PeerConnection::constructor;

void CreateOfferObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  PeerConnection::SdpEvent* data = new PeerConnection::SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateOfferObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_ERROR, (void*)data);
  TRACE_END;
}

void CreateAnswerObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  PeerConnection::SdpEvent* data = new PeerConnection::SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateAnswerObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_ERROR, (void*)data);
  TRACE_END;
}

void SetLocalDescriptionObserver::OnSuccess()
{
  TRACE_CALL;
  parent->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_SUCCESS, static_cast<void*>(NULL));
  TRACE_END;
}

void SetLocalDescriptionObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
}

void SetRemoteDescriptionObserver::OnSuccess()
{
  TRACE_CALL;
  parent->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_SUCCESS, static_cast<void*>(NULL));
  TRACE_END;
}

void SetRemoteDescriptionObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
}

//
// PeerConnection
//

PeerConnection::PeerConnection()
: loop(uv_default_loop())
{
  uv_mutex_init(&lock);
  uv_async_init(loop, &async, Run);

  async.data = this;

  _createOfferObserver = new talk_base::RefCountedObject<CreateOfferObserver>( this );
  _createAnswerObserver = new talk_base::RefCountedObject<CreateAnswerObserver>( this );
  _setLocalDescriptionObserver = new talk_base::RefCountedObject<SetLocalDescriptionObserver>( this );
  _setRemoteDescriptionObserver = new talk_base::RefCountedObject<SetRemoteDescriptionObserver>( this );

  _signalThread = new talk_base::Thread;
  _workerThread = new talk_base::Thread;

  _signalThread->Start();
  _workerThread->Start();

  webrtc::PeerConnectionInterface::IceServer iceServer;
  iceServer.uri = "stun:stun.l.google.com:19302";
  _iceServers.push_back(iceServer);

  webrtc::FakeConstraints constraints;
  constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, webrtc::MediaConstraintsInterface::kValueFalse);
  // constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableSctpDataChannels, true);
  constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableRtpDataChannels, webrtc::MediaConstraintsInterface::kValueTrue);
  constraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, webrtc::MediaConstraintsInterface::kValueFalse);
  constraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, webrtc::MediaConstraintsInterface::kValueFalse);

  _peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
      _signalThread, _workerThread, NULL, NULL, NULL );
  _internalPeerConnection = _peerConnectionFactory->CreatePeerConnection(_iceServers, &constraints, NULL, this);
}

PeerConnection::~PeerConnection()
{
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::QueueEvent(AsyncEventType type, void* data)
{
  TRACE_CALL;
  AsyncEvent evt;
  evt.type = type;
  evt.data = data;
  uv_mutex_lock(&lock);
  _events.push(evt);
  uv_mutex_unlock(&lock);

  uv_async_send(&async);
  TRACE_END;
}

void PeerConnection::Run(uv_async_t* handle, int status)
{
  TRACE_CALL;
  NanScope();

  PeerConnection* self = static_cast<PeerConnection*>(handle->data);
  v8::Handle<v8::Object> pc = NanObjectWrapHandle(self);
  bool do_shutdown = false;

  while(true)
  {
    uv_mutex_lock(&self->lock);
    bool empty = self->_events.empty();
    if(empty)
    {
      uv_mutex_unlock(&self->lock);
      break;
    }
    AsyncEvent evt = self->_events.front();
    self->_events.pop();
    uv_mutex_unlock(&self->lock);

    TRACE_U("evt.type", evt.type);
    if(PeerConnection::ERROR_EVENT & evt.type)
    {
      PeerConnection::ErrorEvent* data = static_cast<PeerConnection::ErrorEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onerror")));
      v8::Local<v8::Value> argv[1];
      argv[0] = Exception::Error(String::New(data->msg.c_str()));
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::SDP_EVENT & evt.type)
    {
      PeerConnection::SdpEvent* data = static_cast<PeerConnection::SdpEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[1];
      argv[0] = String::New(data->desc.c_str());
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::VOID_EVENT & evt.type)
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[0];
      callback->Call(pc, 0, argv);
    } else if(PeerConnection::SIGNALING_STATE_CHANGE & evt.type)
    {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsignalingstatechange")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = Uint32::New(data->state);
        callback->Call(pc, 1, argv);
      }
      if(webrtc::PeerConnectionInterface::kClosed == data->state) {
        do_shutdown = true;
      }
    } else if(PeerConnection::ICE_CONNECTION_STATE_CHANGE & evt.type)
    {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("oniceconnectionstatechange")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = Uint32::New(data->state);
        callback->Call(pc, 1, argv);
      }
    } else if(PeerConnection::ICE_GATHERING_STATE_CHANGE & evt.type)
    {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onicegatheringstatechange")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = Uint32::New(data->state);
        callback->Call(pc, 1, argv);
      }
    } else if(PeerConnection::ICE_CANDIDATE & evt.type)
    {
      PeerConnection::IceEvent* data = static_cast<PeerConnection::IceEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onicecandidate")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[3];
        argv[0] = String::New(data->candidate.c_str());
        argv[1] = String::New(data->sdpMid.c_str());
        argv[2] = Integer::New(data->sdpMLineIndex);
        callback->Call(pc, 3, argv);
      }
    } else if(PeerConnection::NOTIFY_DATA_CHANNEL & evt.type)
    {
      webrtc::DataChannelInterface* dci = static_cast<webrtc::DataChannelInterface*>(evt.data);
      v8::Local<v8::Value> cargv[1];
      cargv[0] = v8::External::New(static_cast<void*>(dci));
      v8::Local<v8::Value> dc = NanPersistentToLocal(DataChannel::constructor)->NewInstance(1, cargv);

      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("ondatachannel")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = dc;
        callback->Call(pc, 1, argv);
      }
    } else if(PeerConnection::NOTIFY_ADD_STREAM & evt.type)
    {
      webrtc::MediaStreamInterface* msi = static_cast<webrtc::MediaStreamInterface*>(evt.data);
      v8::Local<v8::Value> cargv[1];
      cargv[0] = v8::External::New(static_cast<void*>(msi));
      v8::Local<v8::Value> ms = NanPersistentToLocal(MediaStream::constructor)->NewInstance(1, cargv);

      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onaddstream")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = ms;
        callback->Call(pc, 1, argv);
      }
    } else if(PeerConnection::NOTIFY_REMOVE_STREAM & evt.type)
    {
      webrtc::MediaStreamInterface* msi = static_cast<webrtc::MediaStreamInterface*>(evt.data);
      v8::Local<v8::Value> cargv[1];
      cargv[0] = v8::External::New(static_cast<void*>(msi));
      v8::Local<v8::Value> ms = NanPersistentToLocal(MediaStream::constructor)->NewInstance(1, cargv);

      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onremovestream")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = ms;
        callback->Call(pc, 1, argv);
      }
    }
    // FIXME: delete event
  }

  if(do_shutdown) {
    self->_signalThread->Stop();
    self->_workerThread->Stop();
    uv_close((uv_handle_t*)(&self->async), NULL);
  }

  TRACE_END;
}

void PeerConnection::OnError() {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState new_state ) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::SIGNALING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState new_state ) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_CONNECTION_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState new_state ) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_GATHERING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnAddStream( webrtc::MediaStreamInterface* media_stream ) {
  TRACE_CALL;
  media_stream->AddRef();
  QueueEvent(PeerConnection::NOTIFY_ADD_STREAM, static_cast<void*>(media_stream));
  TRACE_END;
}

void PeerConnection::OnRemoveStream( webrtc::MediaStreamInterface* media_stream ) {
  TRACE_CALL;
  media_stream->AddRef();
  QueueEvent(PeerConnection::NOTIFY_REMOVE_STREAM, static_cast<void*>(media_stream));
  TRACE_END;
}

void PeerConnection::OnIceCandidate( const webrtc::IceCandidateInterface* candidate ) {
  TRACE_CALL;
  PeerConnection::IceEvent* data = new PeerConnection::IceEvent(candidate);
  QueueEvent(PeerConnection::ICE_CANDIDATE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnDataChannel( webrtc::DataChannelInterface* data_channel ) {
  TRACE_CALL;
  data_channel->AddRef();
  QueueEvent(PeerConnection::NOTIFY_DATA_CHANNEL, static_cast<void*>(data_channel));
  TRACE_END;
}

void PeerConnection::OnRenegotiationNeeded() {
  TRACE_CALL;
  TRACE_END;
}

NAN_METHOD(PeerConnection::New) {
  TRACE_CALL;
  NanScope();

  if(!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the PeerConnection.");
  }

  PeerConnection* obj = new PeerConnection();
  obj->Wrap( args.This() );
  // V8::AdjustAmountOfExternalAllocatedMemory(1024 * 1024);

  TRACE_END;
  NanReturnValue( args.This() );
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  self->_internalPeerConnection->CreateOffer(self->_createOfferObserver, NULL);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  self->_internalPeerConnection->CreateAnswer(self->_createAnswerObserver, NULL);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::Local<v8::Object> desc = v8::Local<v8::Object>::Cast(args[0]);
  v8::String::Utf8Value _type(desc->Get(v8::String::NewSymbol("type"))->ToString());
  v8::String::Utf8Value _sdp(desc->Get(v8::String::NewSymbol("sdp"))->ToString());

  std::string type = *_type;
  std::string sdp = *_sdp;
  webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp);

  self->_internalPeerConnection->SetLocalDescription(self->_setLocalDescriptionObserver, sdi);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::SetRemoteDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::Local<v8::Object> desc = v8::Local<v8::Object>::Cast(args[0]);
  v8::String::Utf8Value _type(desc->Get(v8::String::NewSymbol("type"))->ToString());
  v8::String::Utf8Value _sdp(desc->Get(v8::String::NewSymbol("sdp"))->ToString());

  std::string type = *_type;
  std::string sdp = *_sdp;
  webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp);

  self->_internalPeerConnection->SetRemoteDescription(self->_setRemoteDescriptionObserver, sdi);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::AddIceCandidate) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  Handle<Object> sdp = Handle<Object>::Cast(args[0]);

  v8::String::Utf8Value _candidate(sdp->Get(String::New("candidate"))->ToString());
  std::string candidate = *_candidate;
  v8::String::Utf8Value _sipMid(sdp->Get(String::New("sdpMid"))->ToString());
  std::string sdp_mid = *_sipMid;
  uint32_t sdp_mline_index = sdp->Get(String::New("sdpMLineIndex"))->Uint32Value();

  webrtc::SdpParseError sdpParseError;
  webrtc::IceCandidateInterface* ci = webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, &sdpParseError);

  if(self->_internalPeerConnection->AddIceCandidate(ci))
  {
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_SUCCESS, static_cast<void*>(NULL));
  } else
  {
    PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(std::string("Failed to set ICE candidate."));
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_ERROR, static_cast<void*>(data));
  }

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::CreateDataChannel) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::String::Utf8Value label(args[0]->ToString());
  Handle<Object> dataChannelDict = Handle<Object>::Cast(args[1]);

  webrtc::DataChannelInit dataChannelInit;
  if(dataChannelDict->Has(String::New("id"))) {
    Local<Value> value = dataChannelDict->Get(String::New("id"));
    if(value->IsInt32()) {
      dataChannelInit.id = value->Int32Value();
    }
  }
  if(dataChannelDict->Has(String::New("maxRetransmitTime"))) {
    Local<Value> value = dataChannelDict->Get(String::New("maxRetransmitTime"));
    if(value->IsInt32()) {
      dataChannelInit.maxRetransmitTime = value->Int32Value();
    }
  }
  if(dataChannelDict->Has(String::New("maxRetransmits"))) {
    Local<Value> value = dataChannelDict->Get(String::New("maxRetransmits"));
    if(value->IsInt32()) {
      dataChannelInit.maxRetransmits = value->Int32Value();
    }
  }
  if(dataChannelDict->Has(String::New("negotiated"))) {
    Local<Value> value = dataChannelDict->Get(String::New("negotiated"));
    if(value->IsBoolean()) {
      dataChannelInit.negotiated = value->BooleanValue();
    }
  }
  if(dataChannelDict->Has(String::New("ordered"))) {
    Local<Value> value = dataChannelDict->Get(String::New("ordered"));
    if(value->IsBoolean()) {
      dataChannelInit.ordered = value->BooleanValue();
    }
  }
  if(dataChannelDict->Has(String::New("protocol"))) {
    Local<Value> value = dataChannelDict->Get(String::New("protocol"));
    if(value->IsString()) {
      dataChannelInit.protocol = *String::Utf8Value(value->ToString());
    }
  }

  talk_base::scoped_refptr<webrtc::DataChannelInterface> dciPtr = self->_internalPeerConnection->CreateDataChannel(*label, &dataChannelInit);
  webrtc::DataChannelInterface* dci = dciPtr.get();
  dci->AddRef();
  dciPtr = NULL;

  v8::Local<v8::Value> cargv[1];
  cargv[0] = v8::External::New(static_cast<void*>(dci));
  v8::Local<v8::Value> dc = NanPersistentToLocal(DataChannel::constructor)->NewInstance(1, cargv);

  TRACE_END;
  NanReturnValue(dc);
}

NAN_METHOD(PeerConnection::AddStream) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  MediaStream* ms = ObjectWrap::Unwrap<MediaStream>( args[0]->ToObject() );
  Handle<Object> constraintsDict = Handle<Object>::Cast(args[1]);

  self->_internalPeerConnection->AddStream(ms->GetInterface(),NULL);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::RemoveStream) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  MediaStream* ms = ObjectWrap::Unwrap<MediaStream>( args[0]->ToObject() );

  self->_internalPeerConnection->RemoveStream(ms->GetInterface());

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::GetLocalStreams) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  talk_base::scoped_refptr<webrtc::StreamCollectionInterface> _streams = self->_internalPeerConnection->local_streams();

  v8::Local<v8::Array> array = v8::Array::New(_streams->count());
  for (unsigned int index = 0; index < _streams->count(); index++) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = v8::External::New(static_cast<void*>(_streams->at(index)));
    array->Set(index, NanPersistentToLocal(MediaStream::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  NanReturnValue(array);
}

NAN_METHOD(PeerConnection::GetRemoteStreams) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  talk_base::scoped_refptr<webrtc::StreamCollectionInterface> _streams = self->_internalPeerConnection->remote_streams();

  v8::Local<v8::Array> array = v8::Array::New(_streams->count());
  for (unsigned int index = 0; index < _streams->count(); index++) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = v8::External::New(static_cast<void*>(_streams->at(index)));
    array->Set(index, NanPersistentToLocal(MediaStream::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  NanReturnValue(array);
}

NAN_METHOD(PeerConnection::GetStreamById) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::String::Utf8Value param1(args[0]->ToString());
  std::string _id = std::string(*param1);
  talk_base::scoped_refptr<webrtc::StreamCollectionInterface> _local = self->_internalPeerConnection->local_streams();
  talk_base::scoped_refptr<webrtc::StreamCollectionInterface> _remote = self->_internalPeerConnection->remote_streams();
  webrtc::MediaStreamInterface* stream = _local->find(_id);
  if (!stream) {
      stream = _remote->find(_id);
  }

  TRACE_END;
  if (stream) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = v8::External::New(static_cast<void*>(stream));
    v8::Local<v8::Value> ms = NanPersistentToLocal(MediaStream::constructor)->NewInstance(1, cargv);
    NanReturnValue(ms);
  } else {
    NanReturnValue(Undefined());
  }
}

NAN_METHOD(PeerConnection::UpdateIce) {
  TRACE_CALL;
  NanScope();
  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(PeerConnection::Close) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  self->_internalPeerConnection->Close();

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_GETTER(PeerConnection::GetLocalDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );
  const webrtc::SessionDescriptionInterface* sdi = self->_internalPeerConnection->local_description();

  Handle<Value> value;
  if(NULL == sdi) {
    value = Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = v8::String::New(sdp.c_str());
  }

  TRACE_END;
  NanReturnValue(value);
}

NAN_GETTER(PeerConnection::GetRemoteDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );
  const webrtc::SessionDescriptionInterface* sdi = self->_internalPeerConnection->remote_description();

  Handle<Value> value;
  if(NULL == sdi) {
    value = Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = v8::String::New(sdp.c_str());
  }

  TRACE_END;
  NanReturnValue(value);
}

NAN_GETTER(PeerConnection::GetSignalingState) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );

  webrtc::PeerConnectionInterface::SignalingState state = self->_internalPeerConnection->signaling_state();

  TRACE_END;
  NanReturnValue(Number::New(state));
}

NAN_GETTER(PeerConnection::GetIceConnectionState) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );

  webrtc::PeerConnectionInterface::IceConnectionState state = self->_internalPeerConnection->ice_connection_state();

  TRACE_END;
  NanReturnValue(Number::New(state));
}

NAN_GETTER(PeerConnection::GetIceGatheringState) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );

  webrtc::PeerConnectionInterface::IceGatheringState state = self->_internalPeerConnection->ice_gathering_state();

  TRACE_END;
  NanReturnValue(Number::New(static_cast<uint32_t>(state)));
}

NAN_SETTER(PeerConnection::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void PeerConnection::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "PeerConnection" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createOffer" ),
    FunctionTemplate::New( CreateOffer )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createAnswer" ),
    FunctionTemplate::New( CreateAnswer )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setLocalDescription" ),
    FunctionTemplate::New( SetLocalDescription )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setRemoteDescription" ),
    FunctionTemplate::New( SetRemoteDescription )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "updateIce" ),
    FunctionTemplate::New( UpdateIce )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "addIceCandidate" ),
    FunctionTemplate::New( AddIceCandidate )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createDataChannel" ),
    FunctionTemplate::New( CreateDataChannel )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "getLocalStreams" ),
    FunctionTemplate::New( GetLocalStreams )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "getRemoteStreams" ),
    FunctionTemplate::New( GetRemoteStreams )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "getStreamById" ),
    FunctionTemplate::New( GetStreamById )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "addStream" ),
    FunctionTemplate::New( AddStream )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "removeStream" ),
    FunctionTemplate::New( RemoveStream )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "close" ),
    FunctionTemplate::New( Close )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(String::New("localDescription"), GetLocalDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("remoteDescription"), GetRemoteDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("signalingState"), GetSignalingState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("iceConnectionState"), GetIceConnectionState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("iceGatheringState"), GetIceGatheringState, ReadOnly);

  NanAssignPersistent(Function, constructor,  tpl->GetFunction() );
  exports->Set( String::NewSymbol("PeerConnection"), tpl->GetFunction() );
}
