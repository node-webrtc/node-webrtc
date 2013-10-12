(function() {

var dataChannelSettings = {
  'unreliable': {
        ordered: false,
        maxRetransmits: 0
      },
  /*
  'reliable': {},
  '@control': {
        outOfOrderAllowed: true,
        maxRetransmitNum: 0
      }
  */
};

var pendingDataChannels = {};
var dataChannels = {}

function doHandleError(error)
{
  throw error;
}

function doComplete()
{
  console.log('complete');
}

function doNothing()
{
  console.log('awaiting data channels')
}

var pc = new mozRTCPeerConnection();
pc.onsignalingstatechange = function(state)
{
  console.info('state change', state);
}

doCreateDataChannels(doComplete);

function doCreateDataChannels(cb)
{
  var labels = Object.keys(dataChannelSettings);
  labels.forEach(function(label) {
    var channelOptions = dataChannelSettings[label];
    var channel = pendingDataChannels[label] = pc.createDataChannel(label, channelOptions);
    channel.binaryType = 'arraybuffer';
    channel.onopen = function() {
      console.info('onopen');
      dataChannels[label] = channel;
      delete pendingDataChannels[label];
      if(Object.keys(dataChannels).length === labels.length) {
        cb.apply(undefined, []);
      }
    };
    channel.onclose = function() {
      console.info('onclose');
    }
    channel.onerror = doHandleError;
  });
  doCreateOffer();
}

function doCreateOffer()
{
  pc.createOffer(
    doSetLocalDesc,
    doHandleError
  );
}

function doSetLocalDesc(desc)
{
  pc.setLocalDescription(
    new mozRTCSessionDescription(desc),
    doSendOffer.bind(undefined, desc),
    doHandleError
  );
}

function doSendOffer(offer)
{
  var xhr = new XMLHttpRequest();
  xhr.onload = function()
  {
    if(200 == xhr.status)
    {
      doSetRemoteDesc(JSON.parse(xhr.responseText));
    }
  }
  // xhr.setRequestHeader('Content-Type', "application/json");
  xhr.open('POST', 'http://localhost:9001', true);
  xhr.send(JSON.stringify({type: offer.type, sdp: offer.sdp}));
}

function doSetRemoteDesc(desc)
{
  pc.setRemoteDescription(
    new mozRTCSessionDescription(desc),
    doNothing,
    doHandleError
  );
}

})();