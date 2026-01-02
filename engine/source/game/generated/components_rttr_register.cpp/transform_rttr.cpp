#include "rttr/registration.h"

import zeytin.game.transform;

RTTR_REGISTRATION
{
	rttr::registration::class_<Transform>("Transform")
		(rttr::metadata("is_component", true))
        .constructor<>()(rttr::policy::ctor::as_object)
		.property("entity_id", &Transform::entity_id)(rttr::metadata("is_hidden", true));
}
