#include "src/dictionaries/webrtc/rtp_transceiver_init.h"

#include <iosfwd>
#include <vector>

#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/enums/webrtc/rtp_transceiver_direction.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/interfaces/media_stream.h"

// TODO(mroberts): Remove me.
#include "src/converters/interfaces.h"  // IWYU pragma: keep

namespace node_webrtc {

#define RTP_TRANSCEIVER_INIT_FN CreateRtpTransceiverInit
#define RTP_TRANSCEIVER_INIT_LIST \
  DEFAULT(webrtc::RtpTransceiverDirection, direction, "direction", webrtc::RtpTransceiverDirection::kSendRecv) \
  DEFAULT(std::vector<MediaStream*>, streams, "streams", std::vector<MediaStream*>())

static Validation<webrtc::RtpTransceiverInit> RTP_TRANSCEIVER_INIT_FN(
    const webrtc::RtpTransceiverDirection direction,
    const std::vector<MediaStream*> streams) {
  webrtc::RtpTransceiverInit init;
  init.direction = direction;
  std::vector<std::string> stream_ids;
  for (const auto& stream : streams) {
    stream_ids.emplace_back(stream->stream()->id());
  }
  init.stream_ids = stream_ids;
  return Pure(init);
}

}  // namespace node_webrtc

#define DICT(X) RTP_TRANSCEIVER_INIT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT

