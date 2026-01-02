#include "rttr/registration.h"

import zeytin.game.scale;

RTTR_REGISTRATION
{
	rttr::registration::class_<Vector2Component>("Vector2Component")
		(rttr::metadata("is_component", true))
        .constructor<>()(rttr::policy::ctor::as_object)
		.property("x", &Vector2Component::x)
		.property("y", &Vector2Component::y)
		.property("entity_id", &Vector2Component::entity_id)(rttr::metadata("is_hidden", true));
}
