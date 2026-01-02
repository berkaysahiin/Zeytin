#include "rttr/registration.h"

import zeytin.game.massive_component;

RTTR_REGISTRATION
{
	rttr::registration::class_<MassiveComponent>("MassiveComponent")
		(rttr::metadata("is_component", true))
        .constructor<>()(rttr::policy::ctor::as_object)
		.property("id", &MassiveComponent::id)
		.property("name", &MassiveComponent::name)
		.property("tag", &MassiveComponent::tag)
		.property("posX", &MassiveComponent::posX)
		.property("posY", &MassiveComponent::posY)
		.property("posZ", &MassiveComponent::posZ)
		.property("rotX", &MassiveComponent::rotX)
		.property("rotY", &MassiveComponent::rotY)
		.property("rotZ", &MassiveComponent::rotZ)
		.property("scaleX", &MassiveComponent::scaleX)
		.property("scaleY", &MassiveComponent::scaleY)
		.property("scaleZ", &MassiveComponent::scaleZ)
		.property("dynamic", &MassiveComponent::dynamic)
		.property("mass", &MassiveComponent::mass)
		.property("friction", &MassiveComponent::friction)
		.property("restitution", &MassiveComponent::restitution)
		.property("gravityEnabled", &MassiveComponent::gravityEnabled)
		.property("visible", &MassiveComponent::visible)
		.property("renderLayer", &MassiveComponent::renderLayer)
		.property("opacity", &MassiveComponent::opacity)
		.property("animated", &MassiveComponent::animated)
		.property("animationTime", &MassiveComponent::animationTime)
		.property("animationSpeed", &MassiveComponent::animationSpeed)
		.property("active", &MassiveComponent::active)
		.property("state", &MassiveComponent::state)
		.property("awarenessRadius", &MassiveComponent::awarenessRadius)
		.property("aggression", &MassiveComponent::aggression)
		.property("networkId", &MassiveComponent::networkId)
		.property("replicated", &MassiveComponent::replicated)
		.property("lastSyncTime", &MassiveComponent::lastSyncTime)
		.property("debugSelected", &MassiveComponent::debugSelected)
		.property("debugValue", &MassiveComponent::debugValue)
		.property("debugLabel", &MassiveComponent::debugLabel)
		.property("entity_id", &MassiveComponent::entity_id)(rttr::metadata("is_hidden", true));
}
