#include "rttr/registration.h"

import zeytin.game.scale;

RTTR_REGISTRATION
{
	rttr::registration::class_<Scale>("Scale")
		(rttr::metadata("is_component", true))
        .constructor<>()(rttr::policy::ctor::as_object)
		.property("x", &Scale::x)
		.property("y", &Scale::y)
		.property("debug_name", &Scale::debug_name)
		.property("test", &Scale::test)
		.property("debug", &Scale::debug)
		.property("entity_id", &Scale::entity_id)(rttr::metadata("is_hidden", true));
}
