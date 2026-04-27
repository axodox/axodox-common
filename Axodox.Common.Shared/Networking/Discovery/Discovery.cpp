#include "common_includes.h"
#include "Discovery.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace {
  const uint64_t magic = 6247212169871777206;
}

namespace Axodox::Networking
{
  Infrastructure::type_registry<discovery_message> discovery_message::derived_types =
    type_registry<discovery_message>::create<discovery_request, discovery_response>();

  void discovery_message::serialize(Storage::stream& stream, Storage::version_t version) const
  {
    stream.write(magic);
  }

  void discovery_message::deserialize(Storage::stream& stream, Storage::version_t /*version*/)
  {
    if (magic != stream.read<uint64_t>()) throw runtime_error("Discovery protocol magic number does not match!");
  }

  discovery_message_kind discovery_request::type() const
  {
    return discovery_message_kind::discovery_request;
  }

  void discovery_request::serialize(Storage::stream& stream, Storage::version_t version) const
  {
    discovery_message::serialize(stream, version);
    stream.write(id);
  }

  void discovery_request::deserialize(Storage::stream& stream, Storage::version_t version)
  {
    discovery_message::deserialize(stream, version);
    stream.read(id);
  }

  discovery_message_kind discovery_response::type() const
  {
    return discovery_message_kind::discovery_response;
  }

  void discovery_response::serialize(Storage::stream& stream, Storage::version_t version) const
  {
    discovery_message::serialize(stream, version);
    stream.write(id);
    Storage::serialize(stream, address, version);
  }

  void discovery_response::deserialize(Storage::stream& stream, Storage::version_t version)
  {
    discovery_message::deserialize(stream, version);
    stream.read(id);
    Storage::deserialize(stream, address, version);
  }
}