export module zeytin.component.registry;

import zeytin.component;
import zeytin.component.document;
import zeytin.component.instance;
import zeytin.utility.typedefs;

export {
    using ComponentDocumentID = uint64;

    void initialize_component_registry();

    ComponentID sync_component(const ComponentDocument& document);
    bool remove_component(ComponentDocumentID document_id);

    MaybeRef<Component> get_component(ComponentID component_id);
    MaybeRef<const Component> get_component_const(ComponentID component_id);

    MaybeRef<Component> get_component_from_document(ComponentDocumentID document_id);
    MaybeRef<const Component> get_component_from_document_const(ComponentDocumentID document_id);

    MaybeRef<Component> get_component_by_name(const String& name);
    MaybeRef<const Component> get_component_by_name_const(const String& name);
    
    List<ComponentID> get_component_ids();
    List<ComponentDocumentID> get_document_ids();


    MaybeRef<ComponentDocument> get_document(ComponentDocumentID document_id);
    MaybeRef<const ComponentDocument> get_document_const(ComponentDocumentID document_id);
    MaybeRef<ComponentDocument> get_document_by_name(const String& name);
    MaybeRef<const ComponentDocument> get_document_by_name_const(const String& name);
    
    Maybe<ComponentDocumentID> get_document_id_for_component(ComponentID component_id);
    MaybeRef<ComponentDocument> get_document_from_component(ComponentID component_id);
    MaybeRef<const ComponentDocument> get_document_from_component_const(ComponentID component_id);

    MaybeRef<ComponentInstance> create_instance(ComponentID component_id);

    MaybeRef<ComponentInstance> get_instance(ComponentInstanceID id);
    MaybeRef<const ComponentInstance> get_instance_const(ComponentInstanceID id);

    bool destroy_instance(ComponentInstanceID id);

    const List<ComponentInstanceID>& get_instances_for(ComponentID component_id);
    List<ComponentInstanceID> get_instances_for_copy(ComponentID component_id);
}
