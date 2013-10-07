[ Constructor (RTCConfiguration configuration, optional MediaConstraints constraints)]
interface RTCPeerConnection : EventTarget  {
    // DONE
    void                  createOffer (RTCSessionDescriptionCallback successCallback, RTCPeerConnectionErrorCallback failureCallback, optional MediaConstraints constraints);

    void                  createAnswer (RTCSessionDescriptionCallback successCallback, RTCPeerConnectionErrorCallback failureCallback, optional MediaConstraints constraints);

    // DONE
    void                  setLocalDescription (RTCSessionDescription description, VoidFunction successCallback, RTCPeerConnectionErrorCallback failureCallback);

    // DONE
    readonly    attribute RTCSessionDescription? localDescription;

    // DONE
    void                  setRemoteDescription (RTCSessionDescription description, VoidFunction successCallback, RTCPeerConnectionErrorCallback failureCallback);

    // DONE
    readonly    attribute RTCSessionDescription? remoteDescription;

    // DONE
    readonly    attribute RTCSignalingState      signalingState;

    void                  updateIce (optional RTCConfiguration configuration, optional MediaConstraints constraints);

    void                  addIceCandidate (RTCIceCandidate candidate, VoidFunction successCallback, RTCPeerConnectionErrorCallback failureCallback);

    readonly    attribute RTCIceGatheringState   iceGatheringState;

    readonly    attribute RTCIceConnectionState  iceConnectionState;

    sequence<MediaStream> getLocalStreams ();

    sequence<MediaStream> getRemoteStreams ();

    MediaStream?          getStreamById (DOMString streamId);

    void                  addStream (MediaStream stream, optional MediaConstraints constraints);

    void                  removeStream (MediaStream stream);

    void                  close ();

                attribute EventHandler           onnegotiationneeded;
                attribute EventHandler           onicecandidate;
                attribute EventHandler           onsignalingstatechange;
                attribute EventHandler           onaddstream;
                attribute EventHandler           onremovestream;
                attribute EventHandler           oniceconnectionstatechange;
};