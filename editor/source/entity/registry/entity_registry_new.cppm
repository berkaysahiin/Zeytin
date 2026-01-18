module;

export module zeytin.entity.registry.runtime;

import zeytin.entity;
import zeytin.entity.document;
import zeytin.level;
import zeytin.utility.typedefs;

export {
	[[nodiscard]]
    ConstRef<Level> runtime_get_current_level();

	[[nodiscard]]
    List<Level> runtime_get_available_levels();

	[[nodiscard]]
    List<EntityID> runtime_get_entity_ids();

	[[nodiscard]]
    MaybeRef<Entity> runtime_get_entity(EntityID entity_id);

	[[nodiscard]]
    MaybeRef<const Entity> runtime_get_entity_const(EntityID entity_id);

	[[nodiscard]]
    List<EntityDocumentID> runtime_get_entity_document_ids();

	[[nodiscard]]
    MaybeRef<EntityDocument> runtime_get_entity_document(EntityDocumentID document_id);

	[[nodiscard]]
    MaybeRef<const EntityDocument> runtime_get_entity_document_const(EntityDocumentID document_id);

	[[nodiscard]]
    Maybe<EntityDocumentID> runtime_get_document_id_for_entity(EntityID entity_id);

	[[nodiscard]]
    Maybe<EntityID> runtime_get_entity_id_for_document(EntityDocumentID document_id);

    void runtime_load_level(ConstRef<Level>level);
    void initialize_entity_runtime_registry();
    void shutdown_entity_runtime_registry();
}
