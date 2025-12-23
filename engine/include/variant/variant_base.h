#pragma once

#include "rttr/registration.h"
#include "variant/variant_macros.h"
#include <functional>
#include "core/raylib_wrapper.h"

struct VariantCreateInfo {
    entity_id entity_id;

    RTTR_ENABLE();
};

struct VariantBase {
    VariantBase() = default;
    VariantBase(VariantCreateInfo info) : entity_id(info.entity_id) {}

    virtual void on_init() {}
    virtual void on_post_init() {}
    virtual void on_update() {}
    virtual void on_play_start() {}
    virtual void on_play_late_start() {}
    virtual void on_play_update() {}
    virtual void on_play_late_update() {}

    uint64_t get_id() { return entity_id; }
    const uint64_t get_id() const { return entity_id; }

    entity_id entity_id;
    bool is_dead = false;
    bool post_inited = false;

    RTTR_ENABLE();
};

#define BEGIN_ZEYTIN(ClassName)   														\
	static void ___register()       													\
    { 															                        \
		using Type = ClassName; 														\
        rttr::registration::class_<PlayerRenderer>( #ClassName ) 						\ 
        	.constructor<>()(rttr::policy::ctor::as_object)                             \
            .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)

#define PROPERTY_(Name) .property( #Name , & Type :: Name)
#define META(Key, Value) (rttr::metadata( #Key , Value ))

#define END_ZEYTIN ; }                                                                  \
	namespace                                                                           \
	{                                                                                   \
	    struct rttr__auto__register__                                                   \
	    {                                                                               \
	        rttr__auto__register__()                                                    \
	        {                                                                           \
	            ___register();                                                          \
	        }                                                                           \
	    };                                                                              \
	}                                                                                   \
	static const rttr__auto__register__ RTTR_CAT(auto_register__, __LINE__);            \


