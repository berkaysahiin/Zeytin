#include "rttr/registration.h"

import zeytin.component;
import zeytin.game.position;

RTTR_REGISTRATION
{
    rttr::registration::class_<Component>("Component")
        .constructor<>()(rttr::policy::ctor::as_object);

	rttr::registration::class_<Position>("Position")
		(rttr::metadata("is_component", true))
        .constructor<>()(rttr::policy::ctor::as_object)
		.property("x", &Position::x)
		.property("y", &Position::y)
		.property("name", &Position::name)
		.property("entity_id", &Position::entity_id)(rttr::metadata("is_hidden", true));
}
