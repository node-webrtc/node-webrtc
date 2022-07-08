export interface RTCAnswerOptions extends RTCOfferAnswerOptions {
}

export interface RTCCertificateExpiration {
    expires?: number;
}

export interface RTCConfiguration {
    bundlePolicy?: RTCBundlePolicy;
    certificates?: RTCCertificate[];
    iceCandidatePoolSize?: number;
    iceServers?: RTCIceServer[];
    iceTransportPolicy?: RTCIceTransportPolicy;
    rtcpMuxPolicy?: RTCRtcpMuxPolicy;
}

export interface RTCDTMFToneChangeEventInit extends EventInit {
    tone?: string;
}

export interface RTCDataChannelEventInit extends EventInit {
    channel: RTCDataChannel;
}

export interface RTCDataChannelInit {
    id?: number;
    maxPacketLifeTime?: number;
    maxRetransmits?: number;
    negotiated?: boolean;
    ordered?: boolean;
    protocol?: string;
}

export interface RTCDtlsFingerprint {
    algorithm?: string;
    value?: string;
}

export interface RTCEncodedAudioFrameMetadata {
    contributingSources?: number[];
    synchronizationSource?: number;
}

export interface RTCEncodedVideoFrameMetadata {
    contributingSources?: number[];
    dependencies?: number[];
    frameId?: number;
    height?: number;
    spatialIndex?: number;
    synchronizationSource?: number;
    temporalIndex?: number;
    width?: number;
}

export interface RTCErrorEventInit extends EventInit {
    error: RTCError;
}

export interface RTCErrorInit {
    errorDetail: RTCErrorDetailType;
    httpRequestStatusCode?: number;
    receivedAlert?: number;
    sctpCauseCode?: number;
    sdpLineNumber?: number;
    sentAlert?: number;
}

export interface RTCIceCandidateInit {
    candidate?: string;
    sdpMLineIndex?: number | null;
    sdpMid?: string | null;
    usernameFragment?: string | null;
}

export interface RTCIceCandidatePairStats extends RTCStats {
    availableIncomingBitrate?: number;
    availableOutgoingBitrate?: number;
    bytesReceived?: number;
    bytesSent?: number;
    currentRoundTripTime?: number;
    localCandidateId: string;
    nominated?: boolean;
    remoteCandidateId: string;
    requestsReceived?: number;
    requestsSent?: number;
    responsesReceived?: number;
    responsesSent?: number;
    state: RTCStatsIceCandidatePairState;
    totalRoundTripTime?: number;
    transportId: string;
}

export interface RTCIceServer {
    credential?: string;
    credentialType?: RTCIceCredentialType;
    urls: string | string[];
    username?: string;
}

export interface RTCInboundRtpStreamStats extends RTCReceivedRtpStreamStats {
    firCount?: number;
    framesDecoded?: number;
    nackCount?: number;
    pliCount?: number;
    qpSum?: number;
    remoteId?: string;
}

export interface RTCLocalSessionDescriptionInit {
    sdp?: string;
    type?: RTCSdpType;
}

export interface RTCOfferAnswerOptions {
}

export interface RTCOfferOptions extends RTCOfferAnswerOptions {
    iceRestart?: boolean;
    offerToReceiveAudio?: boolean;
    offerToReceiveVideo?: boolean;
}

export interface RTCOutboundRtpStreamStats extends RTCSentRtpStreamStats {
    firCount?: number;
    framesEncoded?: number;
    nackCount?: number;
    pliCount?: number;
    qpSum?: number;
    remoteId?: string;
}

export interface RTCPeerConnectionIceErrorEventInit extends EventInit {
    address?: string | null;
    errorCode: number;
    errorText?: string;
    port?: number | null;
    url?: string;
}

export interface RTCPeerConnectionIceEventInit extends EventInit {
    candidate?: RTCIceCandidate | null;
    url?: string | null;
}

export interface RTCReceivedRtpStreamStats extends RTCRtpStreamStats {
    jitter?: number;
    packetsDiscarded?: number;
    packetsLost?: number;
    packetsReceived?: number;
}

export interface RTCRtcpParameters {
    cname?: string;
    reducedSize?: boolean;
}

export interface RTCRtpCapabilities {
    codecs: RTCRtpCodecCapability[];
    headerExtensions: RTCRtpHeaderExtensionCapability[];
}

export interface RTCRtpCodecCapability {
    channels?: number;
    clockRate: number;
    mimeType: string;
    sdpFmtpLine?: string;
}

export interface RTCRtpCodecParameters {
    channels?: number;
    clockRate: number;
    mimeType: string;
    payloadType: number;
    sdpFmtpLine?: string;
}

export interface RTCRtpCodingParameters {
    rid?: string;
}

export interface RTCRtpContributingSource {
    audioLevel?: number;
    rtpTimestamp: number;
    source: number;
    timestamp: DOMHighResTimeStamp;
}

export interface RTCRtpEncodingParameters extends RTCRtpCodingParameters {
    active?: boolean;
    maxBitrate?: number;
    priority?: RTCPriorityType;
    scaleResolutionDownBy?: number;
}

export interface RTCRtpHeaderExtensionCapability {
    uri?: string;
}

export interface RTCRtpHeaderExtensionParameters {
    encrypted?: boolean;
    id: number;
    uri: string;
}

export interface RTCRtpParameters {
    codecs: RTCRtpCodecParameters[];
    headerExtensions: RTCRtpHeaderExtensionParameters[];
    rtcp: RTCRtcpParameters;
}

export interface RTCRtpReceiveParameters extends RTCRtpParameters {
}

export interface RTCRtpSendParameters extends RTCRtpParameters {
    degradationPreference?: RTCDegradationPreference;
    encodings: RTCRtpEncodingParameters[];
    transactionId: string;
}

export interface RTCRtpStreamStats extends RTCStats {
    codecId?: string;
    kind: string;
    ssrc: number;
    transportId?: string;
}

export interface RTCRtpSynchronizationSource extends RTCRtpContributingSource {
}

export interface RTCRtpTransceiverInit {
    direction?: RTCRtpTransceiverDirection;
    sendEncodings?: RTCRtpEncodingParameters[];
    streams?: MediaStream[];
}

export interface RTCSentRtpStreamStats extends RTCRtpStreamStats {
    bytesSent?: number;
    packetsSent?: number;
}

export interface RTCSessionDescriptionInit {
    sdp?: string;
    type: RTCSdpType;
}

export interface RTCStats {
    id: string;
    timestamp: DOMHighResTimeStamp;
    type: RTCStatsType;
}

export interface RTCTrackEventInit extends EventInit {
    receiver: RTCRtpReceiver;
    streams?: MediaStream[];
    track: MediaStreamTrack;
    transceiver: RTCRtpTransceiver;
}

export interface RTCTransportStats extends RTCStats {
    bytesReceived?: number;
    bytesSent?: number;
    dtlsCipher?: string;
    dtlsState: RTCDtlsTransportState;
    localCertificateId?: string;
    remoteCertificateId?: string;
    rtcpTransportStatsId?: string;
    selectedCandidatePairId?: string;
    srtpCipher?: string;
    tlsVersion?: string;
}
export interface RTCCertificate {
    readonly expires: EpochTimeStamp;
    getFingerprints(): RTCDtlsFingerprint[];
}

declare var RTCCertificate: {
    prototype: RTCCertificate;
    new(): RTCCertificate;
};

export interface RTCDTMFSenderEventMap {
    "tonechange": RTCDTMFToneChangeEvent;
}

export interface RTCDTMFSender extends EventTarget {
    readonly canInsertDTMF: boolean;
    ontonechange: ((this: RTCDTMFSender, ev: RTCDTMFToneChangeEvent) => any) | null;
    readonly toneBuffer: string;
    insertDTMF(tones: string, duration?: number, interToneGap?: number): void;
    addEventListener<K extends keyof RTCDTMFSenderEventMap>(type: K, listener: (this: RTCDTMFSender, ev: RTCDTMFSenderEventMap[K]) => any, options?: boolean | AddEventListenerOptions): void;
    addEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void;
    removeEventListener<K extends keyof RTCDTMFSenderEventMap>(type: K, listener: (this: RTCDTMFSender, ev: RTCDTMFSenderEventMap[K]) => any, options?: boolean | EventListenerOptions): void;
    removeEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | EventListenerOptions): void;
}

declare var RTCDTMFSender: {
    prototype: RTCDTMFSender;
    new(): RTCDTMFSender;
};

/** Events sent to indicate that DTMF tones have started or finished playing. This interface is used by the tonechange event. */
export interface RTCDTMFToneChangeEvent extends Event {
    readonly tone: string;
}

declare var RTCDTMFToneChangeEvent: {
    prototype: RTCDTMFToneChangeEvent;
    new(type: string, eventInitDict?: RTCDTMFToneChangeEventInit): RTCDTMFToneChangeEvent;
};

export interface RTCDataChannelEventMap {
    "bufferedamountlow": Event;
    "close": Event;
    "closing": Event;
    "error": Event;
    "message": MessageEvent;
    "open": Event;
}

export interface RTCDataChannel extends EventTarget {
    binaryType: BinaryType;
    readonly bufferedAmount: number;
    bufferedAmountLowThreshold: number;
    readonly id: number | null;
    readonly label: string;
    readonly maxPacketLifeTime: number | null;
    readonly maxRetransmits: number | null;
    readonly negotiated: boolean;
    onbufferedamountlow: ((this: RTCDataChannel, ev: Event) => any) | null;
    onclose: ((this: RTCDataChannel, ev: Event) => any) | null;
    onclosing: ((this: RTCDataChannel, ev: Event) => any) | null;
    onerror: ((this: RTCDataChannel, ev: Event) => any) | null;
    onmessage: ((this: RTCDataChannel, ev: MessageEvent) => any) | null;
    onopen: ((this: RTCDataChannel, ev: Event) => any) | null;
    readonly ordered: boolean;
    readonly protocol: string;
    readonly readyState: RTCDataChannelState;
    close(): void;
    send(data: string): void;
    send(data: Blob): void;
    send(data: ArrayBuffer): void;
    send(data: ArrayBufferView): void;
    addEventListener<K extends keyof RTCDataChannelEventMap>(type: K, listener: (this: RTCDataChannel, ev: RTCDataChannelEventMap[K]) => any, options?: boolean | AddEventListenerOptions): void;
    addEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void;
    removeEventListener<K extends keyof RTCDataChannelEventMap>(type: K, listener: (this: RTCDataChannel, ev: RTCDataChannelEventMap[K]) => any, options?: boolean | EventListenerOptions): void;
    removeEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | EventListenerOptions): void;
}

declare var RTCDataChannel: {
    prototype: RTCDataChannel;
    new(): RTCDataChannel;
};

export interface RTCDataChannelEvent extends Event {
    readonly channel: RTCDataChannel;
}

declare var RTCDataChannelEvent: {
    prototype: RTCDataChannelEvent;
    new(type: string, eventInitDict: RTCDataChannelEventInit): RTCDataChannelEvent;
};

export interface RTCDtlsTransportEventMap {
    "error": Event;
    "statechange": Event;
}

export interface RTCDtlsTransport extends EventTarget {
    readonly iceTransport: RTCIceTransport;
    onerror: ((this: RTCDtlsTransport, ev: Event) => any) | null;
    onstatechange: ((this: RTCDtlsTransport, ev: Event) => any) | null;
    readonly state: RTCDtlsTransportState;
    getRemoteCertificates(): ArrayBuffer[];
    addEventListener<K extends keyof RTCDtlsTransportEventMap>(type: K, listener: (this: RTCDtlsTransport, ev: RTCDtlsTransportEventMap[K]) => any, options?: boolean | AddEventListenerOptions): void;
    addEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void;
    removeEventListener<K extends keyof RTCDtlsTransportEventMap>(type: K, listener: (this: RTCDtlsTransport, ev: RTCDtlsTransportEventMap[K]) => any, options?: boolean | EventListenerOptions): void;
    removeEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | EventListenerOptions): void;
}

declare var RTCDtlsTransport: {
    prototype: RTCDtlsTransport;
    new(): RTCDtlsTransport;
};

export interface RTCEncodedAudioFrame {
    data: ArrayBuffer;
    readonly timestamp: number;
    getMetadata(): RTCEncodedAudioFrameMetadata;
}

declare var RTCEncodedAudioFrame: {
    prototype: RTCEncodedAudioFrame;
    new(): RTCEncodedAudioFrame;
};

export interface RTCEncodedVideoFrame {
    data: ArrayBuffer;
    readonly timestamp: number;
    readonly type: RTCEncodedVideoFrameType;
    getMetadata(): RTCEncodedVideoFrameMetadata;
}

declare var RTCEncodedVideoFrame: {
    prototype: RTCEncodedVideoFrame;
    new(): RTCEncodedVideoFrame;
};

export interface RTCError extends DOMException {
    readonly errorDetail: RTCErrorDetailType;
    readonly receivedAlert: number | null;
    readonly sctpCauseCode: number | null;
    readonly sdpLineNumber: number | null;
    readonly sentAlert: number | null;
}

declare var RTCError: {
    prototype: RTCError;
    new(init: RTCErrorInit, message?: string): RTCError;
};

export interface RTCErrorEvent extends Event {
    readonly error: RTCError;
}

declare var RTCErrorEvent: {
    prototype: RTCErrorEvent;
    new(type: string, eventInitDict: RTCErrorEventInit): RTCErrorEvent;
};

/** The RTCIceCandidate interface—part of the WebRTC API—represents a candidate Internet Connectivity Establishment (ICE) configuration which may be used to establish an RTCPeerConnection. */
export interface RTCIceCandidate {
    readonly address: string | null;
    readonly candidate: string;
    readonly component: RTCIceComponent | null;
    readonly foundation: string | null;
    readonly port: number | null;
    readonly priority: number | null;
    readonly protocol: RTCIceProtocol | null;
    readonly relatedAddress: string | null;
    readonly relatedPort: number | null;
    readonly sdpMLineIndex: number | null;
    readonly sdpMid: string | null;
    readonly tcpType: RTCIceTcpCandidateType | null;
    readonly type: RTCIceCandidateType | null;
    readonly usernameFragment: string | null;
    toJSON(): RTCIceCandidateInit;
}

declare var RTCIceCandidate: {
    prototype: RTCIceCandidate;
    new(candidateInitDict?: RTCIceCandidateInit): RTCIceCandidate;
};

export interface RTCIceTransportEventMap {
    "gatheringstatechange": Event;
    "statechange": Event;
}

/** Provides access to information about the ICE transport layer over which the data is being sent and received. */
export interface RTCIceTransport extends EventTarget {
    readonly gatheringState: RTCIceGathererState;
    ongatheringstatechange: ((this: RTCIceTransport, ev: Event) => any) | null;
    onstatechange: ((this: RTCIceTransport, ev: Event) => any) | null;
    readonly state: RTCIceTransportState;
    addEventListener<K extends keyof RTCIceTransportEventMap>(type: K, listener: (this: RTCIceTransport, ev: RTCIceTransportEventMap[K]) => any, options?: boolean | AddEventListenerOptions): void;
    addEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void;
    removeEventListener<K extends keyof RTCIceTransportEventMap>(type: K, listener: (this: RTCIceTransport, ev: RTCIceTransportEventMap[K]) => any, options?: boolean | EventListenerOptions): void;
    removeEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | EventListenerOptions): void;
}

declare var RTCIceTransport: {
    prototype: RTCIceTransport;
    new(): RTCIceTransport;
};

export interface RTCPeerConnectionEventMap {
    "connectionstatechange": Event;
    "datachannel": RTCDataChannelEvent;
    "icecandidate": RTCPeerConnectionIceEvent;
    "icecandidateerror": Event;
    "iceconnectionstatechange": Event;
    "icegatheringstatechange": Event;
    "negotiationneeded": Event;
    "signalingstatechange": Event;
    "track": RTCTrackEvent;
}

/** A WebRTC connection between the local computer and a remote peer. It provides methods to connect to a remote peer, maintain and monitor the connection, and close the connection once it's no longer needed. */
export interface RTCPeerConnection extends EventTarget {
    readonly canTrickleIceCandidates: boolean | null;
    readonly connectionState: RTCPeerConnectionState;
    readonly currentLocalDescription: RTCSessionDescription | null;
    readonly currentRemoteDescription: RTCSessionDescription | null;
    readonly iceConnectionState: RTCIceConnectionState;
    readonly iceGatheringState: RTCIceGatheringState;
    readonly localDescription: RTCSessionDescription | null;
    onconnectionstatechange: ((this: RTCPeerConnection, ev: Event) => any) | null;
    ondatachannel: ((this: RTCPeerConnection, ev: RTCDataChannelEvent) => any) | null;
    onicecandidate: ((this: RTCPeerConnection, ev: RTCPeerConnectionIceEvent) => any) | null;
    onicecandidateerror: ((this: RTCPeerConnection, ev: Event) => any) | null;
    oniceconnectionstatechange: ((this: RTCPeerConnection, ev: Event) => any) | null;
    onicegatheringstatechange: ((this: RTCPeerConnection, ev: Event) => any) | null;
    onnegotiationneeded: ((this: RTCPeerConnection, ev: Event) => any) | null;
    onsignalingstatechange: ((this: RTCPeerConnection, ev: Event) => any) | null;
    ontrack: ((this: RTCPeerConnection, ev: RTCTrackEvent) => any) | null;
    readonly pendingLocalDescription: RTCSessionDescription | null;
    readonly pendingRemoteDescription: RTCSessionDescription | null;
    readonly remoteDescription: RTCSessionDescription | null;
    readonly sctp: RTCSctpTransport | null;
    readonly signalingState: RTCSignalingState;
    addIceCandidate(candidate?: RTCIceCandidateInit): Promise<void>;
    /** @deprecated */
    addIceCandidate(candidate: RTCIceCandidateInit, successCallback: VoidFunction, failureCallback: RTCPeerConnectionErrorCallback): Promise<void>;
    addTrack(track: MediaStreamTrack, ...streams: MediaStream[]): RTCRtpSender;
    addTransceiver(trackOrKind: MediaStreamTrack | string, init?: RTCRtpTransceiverInit): RTCRtpTransceiver;
    close(): void;
    createAnswer(options?: RTCAnswerOptions): Promise<RTCSessionDescriptionInit>;
    /** @deprecated */
    createAnswer(successCallback: RTCSessionDescriptionCallback, failureCallback: RTCPeerConnectionErrorCallback): Promise<void>;
    createDataChannel(label: string, dataChannelDict?: RTCDataChannelInit): RTCDataChannel;
    createOffer(options?: RTCOfferOptions): Promise<RTCSessionDescriptionInit>;
    /** @deprecated */
    createOffer(successCallback: RTCSessionDescriptionCallback, failureCallback: RTCPeerConnectionErrorCallback, options?: RTCOfferOptions): Promise<void>;
    getConfiguration(): RTCConfiguration;
    getReceivers(): RTCRtpReceiver[];
    getSenders(): RTCRtpSender[];
    getStats(selector?: MediaStreamTrack | null): Promise<RTCStatsReport>;
    getTransceivers(): RTCRtpTransceiver[];
    removeTrack(sender: RTCRtpSender): void;
    restartIce(): void;
    setConfiguration(configuration?: RTCConfiguration): void;
    setLocalDescription(description?: RTCLocalSessionDescriptionInit): Promise<void>;
    /** @deprecated */
    setLocalDescription(description: RTCLocalSessionDescriptionInit, successCallback: VoidFunction, failureCallback: RTCPeerConnectionErrorCallback): Promise<void>;
    setRemoteDescription(description: RTCSessionDescriptionInit): Promise<void>;
    /** @deprecated */
    setRemoteDescription(description: RTCSessionDescriptionInit, successCallback: VoidFunction, failureCallback: RTCPeerConnectionErrorCallback): Promise<void>;
    addEventListener<K extends keyof RTCPeerConnectionEventMap>(type: K, listener: (this: RTCPeerConnection, ev: RTCPeerConnectionEventMap[K]) => any, options?: boolean | AddEventListenerOptions): void;
    addEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void;
    removeEventListener<K extends keyof RTCPeerConnectionEventMap>(type: K, listener: (this: RTCPeerConnection, ev: RTCPeerConnectionEventMap[K]) => any, options?: boolean | EventListenerOptions): void;
    removeEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | EventListenerOptions): void;
}

declare var RTCPeerConnection: {
    prototype: RTCPeerConnection;
    new(configuration?: RTCConfiguration): RTCPeerConnection;
    generateCertificate(keygenAlgorithm: AlgorithmIdentifier): Promise<RTCCertificate>;
};

export interface RTCPeerConnectionIceErrorEvent extends Event {
    readonly address: string | null;
    readonly errorCode: number;
    readonly errorText: string;
    readonly port: number | null;
    readonly url: string;
}

declare var RTCPeerConnectionIceErrorEvent: {
    prototype: RTCPeerConnectionIceErrorEvent;
    new(type: string, eventInitDict: RTCPeerConnectionIceErrorEventInit): RTCPeerConnectionIceErrorEvent;
};

/** Events that occurs in relation to ICE candidates with the target, usually an RTCPeerConnection. Only one event is of this type: icecandidate. */
export interface RTCPeerConnectionIceEvent extends Event {
    readonly candidate: RTCIceCandidate | null;
}

declare var RTCPeerConnectionIceEvent: {
    prototype: RTCPeerConnectionIceEvent;
    new(type: string, eventInitDict?: RTCPeerConnectionIceEventInit): RTCPeerConnectionIceEvent;
};

/** This WebRTC API interface manages the reception and decoding of data for a MediaStreamTrack on an RTCPeerConnection. */
export interface RTCRtpReceiver {
    readonly track: MediaStreamTrack;
    readonly transport: RTCDtlsTransport | null;
    getContributingSources(): RTCRtpContributingSource[];
    getParameters(): RTCRtpReceiveParameters;
    getStats(): Promise<RTCStatsReport>;
    getSynchronizationSources(): RTCRtpSynchronizationSource[];
}

declare var RTCRtpReceiver: {
    prototype: RTCRtpReceiver;
    new(): RTCRtpReceiver;
    getCapabilities(kind: string): RTCRtpCapabilities | null;
};

/** Provides the ability to control and obtain details about how a particular MediaStreamTrack is encoded and sent to a remote peer. */
export interface RTCRtpSender {
    readonly dtmf: RTCDTMFSender | null;
    readonly track: MediaStreamTrack | null;
    readonly transport: RTCDtlsTransport | null;
    getParameters(): RTCRtpSendParameters;
    getStats(): Promise<RTCStatsReport>;
    replaceTrack(withTrack: MediaStreamTrack | null): Promise<void>;
    setParameters(parameters: RTCRtpSendParameters): Promise<void>;
    setStreams(...streams: MediaStream[]): void;
}

declare var RTCRtpSender: {
    prototype: RTCRtpSender;
    new(): RTCRtpSender;
    getCapabilities(kind: string): RTCRtpCapabilities | null;
};

export interface RTCRtpTransceiver {
    readonly currentDirection: RTCRtpTransceiverDirection | null;
    direction: RTCRtpTransceiverDirection;
    readonly mid: string | null;
    readonly receiver: RTCRtpReceiver;
    readonly sender: RTCRtpSender;
    setCodecPreferences(codecs: RTCRtpCodecCapability[]): void;
    stop(): void;
}

declare var RTCRtpTransceiver: {
    prototype: RTCRtpTransceiver;
    new(): RTCRtpTransceiver;
};

export interface RTCSctpTransportEventMap {
    "statechange": Event;
}

export interface RTCSctpTransport extends EventTarget {
    readonly maxChannels: number | null;
    readonly maxMessageSize: number;
    onstatechange: ((this: RTCSctpTransport, ev: Event) => any) | null;
    readonly state: RTCSctpTransportState;
    readonly transport: RTCDtlsTransport;
    addEventListener<K extends keyof RTCSctpTransportEventMap>(type: K, listener: (this: RTCSctpTransport, ev: RTCSctpTransportEventMap[K]) => any, options?: boolean | AddEventListenerOptions): void;
    addEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | AddEventListenerOptions): void;
    removeEventListener<K extends keyof RTCSctpTransportEventMap>(type: K, listener: (this: RTCSctpTransport, ev: RTCSctpTransportEventMap[K]) => any, options?: boolean | EventListenerOptions): void;
    removeEventListener(type: string, listener: EventListenerOrEventListenerObject, options?: boolean | EventListenerOptions): void;
}

declare var RTCSctpTransport: {
    prototype: RTCSctpTransport;
    new(): RTCSctpTransport;
};

/** One end of a connection—or potential connection—and how it's configured. Each RTCSessionDescription consists of a description type indicating which part of the offer/answer negotiation process it describes and of the SDP descriptor of the session. */
export interface RTCSessionDescription {
    readonly sdp: string;
    readonly type: RTCSdpType;
    toJSON(): any;
}

declare var RTCSessionDescription: {
    prototype: RTCSessionDescription;
    new(descriptionInitDict: RTCSessionDescriptionInit): RTCSessionDescription;
};

export interface RTCStatsReport {
    forEach(callbackfn: (value: any, key: string, parent: RTCStatsReport) => void, thisArg?: any): void;
}

declare var RTCStatsReport: {
    prototype: RTCStatsReport;
    new(): RTCStatsReport;
};

export interface RTCTrackEvent extends Event {
    readonly receiver: RTCRtpReceiver;
    readonly streams: ReadonlyArray<MediaStream>;
    readonly track: MediaStreamTrack;
    readonly transceiver: RTCRtpTransceiver;
}

declare var RTCTrackEvent: {
    prototype: RTCTrackEvent;
    new(type: string, eventInitDict: RTCTrackEventInit): RTCTrackEvent;
};

export interface RTCPeerConnectionErrorCallback {
    (error: DOMException): void;
}

export interface RTCSessionDescriptionCallback {
    (description: RTCSessionDescriptionInit): void;
}

type RTCBundlePolicy = "balanced" | "max-bundle" | "max-compat";
type RTCDataChannelState = "closed" | "closing" | "connecting" | "open";
type RTCDegradationPreference = "balanced" | "maintain-framerate" | "maintain-resolution";
type RTCDtlsTransportState = "closed" | "connected" | "connecting" | "failed" | "new";
type RTCEncodedVideoFrameType = "delta" | "empty" | "key";
type RTCErrorDetailType = "data-channel-failure" | "dtls-failure" | "fingerprint-failure" | "hardware-encoder-error" | "hardware-encoder-not-available" | "sctp-failure" | "sdp-syntax-error";
type RTCIceCandidateType = "host" | "prflx" | "relay" | "srflx";
type RTCIceComponent = "rtcp" | "rtp";
type RTCIceConnectionState = "checking" | "closed" | "completed" | "connected" | "disconnected" | "failed" | "new";
type RTCIceCredentialType = "password";
type RTCIceGathererState = "complete" | "gathering" | "new";
type RTCIceGatheringState = "complete" | "gathering" | "new";
type RTCIceProtocol = "tcp" | "udp";
type RTCIceTcpCandidateType = "active" | "passive" | "so";
type RTCIceTransportPolicy = "all" | "relay";
type RTCIceTransportState = "checking" | "closed" | "completed" | "connected" | "disconnected" | "failed" | "new";
type RTCPeerConnectionState = "closed" | "connected" | "connecting" | "disconnected" | "failed" | "new";
type RTCPriorityType = "high" | "low" | "medium" | "very-low";
type RTCRtcpMuxPolicy = "require";
type RTCRtpTransceiverDirection = "inactive" | "recvonly" | "sendonly" | "sendrecv" | "stopped";
type RTCSctpTransportState = "closed" | "connected" | "connecting";
type RTCSdpType = "answer" | "offer" | "pranswer" | "rollback";
type RTCSignalingState = "closed" | "have-local-offer" | "have-local-pranswer" | "have-remote-offer" | "have-remote-pranswer" | "stable";
type RTCStatsIceCandidatePairState = "failed" | "frozen" | "in-progress" | "inprogress" | "succeeded" | "waiting";
type RTCStatsType = "candidate-pair" | "certificate" | "codec" | "csrc" | "data-channel" | "inbound-rtp" | "local-candidate" | "media-source" | "outbound-rtp" | "peer-connection" | "remote-candidate" | "remote-inbound-rtp" | "remote-outbound-rtp" | "track" | "transport";
