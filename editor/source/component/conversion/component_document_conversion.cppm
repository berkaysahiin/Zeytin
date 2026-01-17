export module zeytin.component.document.conversion;

import zeytin.component;
import zeytin.component.document;

export {
    Component component_from_document(const ComponentDocument& document);
    ComponentDocument document_from_component(const Component& component);
}
