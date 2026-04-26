#include "common_includes.h"
#include "Discovery.h"

using namespace std;

namespace {
  const uint64_t magic = 6247212169871777206;
}

namespace Axodox::Networking
{
  void discovery_request::serialize(Storage::stream& stream, Storage::version_t /*version*/) const
  {
    stream.write(magic);
    stream.write(id);
  }

  void discovery_request::deserialize(Storage::stream& stream, Storage::version_t /*version*/)
  {
    if (magic != stream.read<uint64_t>()) throw runtime_error("Discovery protocol magic number does not match!");
    stream.read(id);
  }

  void discovery_response::serialize(Storage::stream& stream, Storage::version_t version) const
  {
    stream.write(magic);
    stream.write(id);
    Storage::serialize(stream, address, version);
  }

  void discovery_response::deserialize(Storage::stream& stream, Storage::version_t version)
  {
    if (magic != stream.read<uint64_t>()) throw runtime_error("Discovery protocol magic number does not match!");
    stream.read(id);
    Storage::deserialize(stream, address, version);
  }
}