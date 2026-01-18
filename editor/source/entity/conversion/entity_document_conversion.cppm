export module zeytin.entity.document.conversion;

import zeytin.entity;
import zeytin.entity.document;

export {
    Entity entity_from_document(const EntityDocument& document);
    void document_from_entity(const Entity& entity, EntityDocument& document);
}
