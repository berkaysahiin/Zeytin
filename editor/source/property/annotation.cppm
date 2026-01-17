export module zeytin.annotation;

export import zeytin.common.annotation;
import zeytin.utility.typedefs;

import zeytin.utility.json;

export {
    using AnnotationValue = JsonValue;
    using AnnotationMap = HashMap<AnnotationKey, AnnotationValue>;
}

