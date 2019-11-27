#include "src/dictionaries/webrtc/rtp_transceiver_init.h"

#include <iosfwd>
#include <vector>

#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/rtp_parameters.h>
#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/enums/webrtc/rtp_transceiver_direction.h"  // IWYU pragma: keep
#include "src/dictionaries/webrtc/rtp_encoding_parameters.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/interfaces/media_stream.h"

namespace node_webrtc {

#define RTP_TRANSCEIVER_INIT_FN CreateRtpTransceiverInit
#define RTP_TRANSCEIVER_INIT_LIST \
  DICT_DEFAULT(webrtc::RtpTransceiverDirection, direction, "direction", webrtc::RtpTransceiverDirection::kSendRecv) \
  DICT_DEFAULT(std::vector<MediaStream*>, streams, "streams", std::vector<MediaStream*>()) \
  DICT_DEFAULT(std::vector<webrtc::RtpEncodingParameters>, sendEncodings, "sendEncodings", std::vector<webrtc::RtpEncodingParameters>())

static Validation<webrtc::RtpTransceiverInit> RTP_TRANSCEIVER_INIT_FN(
    const webrtc::RtpTransceiverDirection direction,
    const std::vector<MediaStream*>& streams,
    const std::vector<webrtc::RtpEncodingParameters> sendEncodings) {
  webrtc::RtpTransceiverInit init;
  init.direction = direction;
  std::vector<std::string> stream_ids;
  stream_ids.reserve(streams.size());
  for (const auto& stream : streams) {
    stream_ids.emplace_back(stream->stream()->id());
  }
  init.stream_ids = stream_ids;
  init.send_encodings = sendEncodings;
  return Pure(init);
}

}  // namespace node_webrtc

#define DICT(X) RTP_TRANSCEIVER_INIT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
