#include "rttr/registration.h"

import component.test_rendering;

RTTR_REGISTRATION
{
	rttr::registration::class_<TestRendering>("TestRendering")
		(rttr::metadata("is_component", true))
        .constructor<>()(rttr::policy::ctor::as_object)
		.property("position_x", &TestRendering::position_x)
		.property("position_y", &TestRendering::position_y)
		.property("width", &TestRendering::width)
		.property("height", &TestRendering::height)
		.property("color_r", &TestRendering::color_r)
		.property("color_g", &TestRendering::color_g)
		.property("color_b", &TestRendering::color_b)
		.property("color_a", &TestRendering::color_a)
		.property("is_filled", &TestRendering::is_filled)
		.property("border_thickness", &TestRendering::border_thickness)
		.property("label", &TestRendering::label)
		.property("visible", &TestRendering::visible)
		.property("entity_id", &TestRendering::entity_id)(rttr::metadata("is_hidden", true));
}
