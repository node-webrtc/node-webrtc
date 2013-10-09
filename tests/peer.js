(function() {

function doHandleError(error)
{
  throw error;
}

function doComplete()
{
  console.log('!');
}

var pc = new mozRTCPeerConnection();
pc.onsignalingstatechange = function(state)
{
  console.info('state change', state);
}
var mediaOptions = {
  video: false,
  audio: true,
  fake: true
};
navigator.mozGetUserMedia(mediaOptions,
  function(stream)
  {
    pc.addStream(stream);
    doCreateOffer();
  },
  doHandleError
  );

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
    doComplete,
    doHandleError
  );
}

})();