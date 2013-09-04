
#include "fbx_value.hpp"

namespace FBX {
	/* the linker doesn't like these inlined... */
	Value::BaseContent::~BaseContent() {
	}
	SharedValue::BaseContent::~BaseContent() {
	}
}