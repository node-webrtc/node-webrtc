#include <node.h>
#include <v8.h>

#include "talk/app/webrtc/peerconnectioninterface.h"

#include "common.h"
#include "mediastream.h"
#include "nan.h"

static const char *MEDIA_STREAM_NAME = "node-webrtc";
static const char *AUDIO_TRACK_NAME = "node-webrtc-audio";

static talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory = webrtc::CreatePeerConnectionFactory();

static talk_base::scoped_refptr<webrtc::MediaStreamInterface> getUserMedia(bool audio, bool video) {
  talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream =
    peerConnectionFactory->CreateLocalMediaStream(MEDIA_STREAM_NAME);
  if (audio) {
    talk_base::scoped_refptr<webrtc::AudioTrackInterface> audioTrack(
      peerConnectionFactory->CreateAudioTrack(AUDIO_TRACK_NAME,
        peerConnectionFactory->CreateAudioSource(NULL)));
    stream->AddTrack(audioTrack);
  }
  if (video) {
    // TODO(mroberts): Add video support.
  }
  return stream;
}

NAN_METHOD(GetUserMedia) {
  TRACE_CALL;
  NanScope();

  v8::Local<v8::Object> options = v8::Local<v8::Object>::Cast(args[0]);
  /*v8::Local<v8::Boolean> audio = options->Get(v8::String::NewSymbol("audio"))->ToBoolean();
  v8::Local<v8::Boolean> video = options->Get(v8::String::NewSymbol("video"))->ToBoolean();*/

  /*talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream = getUserMedia(audio->Value(), video->Value());*/
  talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream = getUserMedia(true, false);

  v8::Local<v8::Value> cargv[1];
  cargv[0] = v8::External::New(static_cast<void*>(stream));

  v8::Local<v8::Value> wrapped = NanNew(MediaStream::constructor)->NewInstance(1, cargv);

  TRACE_END;
  NanReturnValue(wrapped);
}
