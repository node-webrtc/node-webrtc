(function() {

function doHandleError(error)
{
  throw error;
}

function doNOP()
{
  console.log('!');
}

var pc = new mozRTCPeerConnection();
var mediaOptions = {
  video: false,
  audio: true
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
  console.log(desc);
  pc.setRemoteDescription(
    new mozRTCSessionDescription(desc),
    doNOP,
    doHandleError
  );
}

})();