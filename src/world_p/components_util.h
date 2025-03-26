// please do not modify this file unless you know exactly what you are doing

#define RFCT_REMOVE_COMPONENT_AT_INDEX(compVecPtr, type, index)\
            switch (type) {\
                case ComponentEnum::nameComponent: {\
                    auto* vec = static_cast<std::vector<nameComponent>*>(compVecPtr);\
                    if (index < vec->size() - 1) {\
                        std::swap(vec->at(index), vec->back());\
                    }\
                    vec->pop_back();\
                    break;\
                }\
                case ComponentEnum::damageComponent: {\
                    auto* vec = static_cast<std::vector<damageComponent>*>(compVecPtr);\
                    if (index < vec->size() - 1) {\
                        std::swap(vec->at(index), vec->back());\
                    }\
                    vec->pop_back();\
                    break;\
                }\
                case ComponentEnum::healthComponent: {\
                    auto* vec = static_cast<std::vector<healthComponent>*>(compVecPtr);\
                    if (index < vec->size() - 1) {\
                        std::swap(vec->at(index), vec->back());\
                    }\
                    vec->pop_back();\
                    break;\
                }\
                default:\
                    RFCT_CRITICAL("Couldn't delete entity components");\
                    break;\
            }