#include "acl_stdafx.hpp"
#include "acl_cpp/mqtt/mqtt_disconnect.hpp"

namespace acl {

mqtt_disconnect::mqtt_disconnect()
: mqtt_message(MQTT_DISCONNECT)
{
}

mqtt_disconnect::~mqtt_disconnect(void) {}

bool mqtt_disconnect::to_string(string& out) {
	bool old_mode = out.get_bin();

	this->set_data_length(0);

	if (!this->pack_header(out)) {
		out.set_bin(old_mode);
		return false;
	}

	out.set_bin(old_mode);
	return true;
}

} // namespace acl