export module zeytin.annotation;

export import zeytin.shared.annotation;
import zeytin.utility.typedefs;

import zeytin.utility.json;

export {
    using AnnotationValue = JsonValue;
    using AnnotationMap = HashMap<AnnotationKey, AnnotationValue>;
}

