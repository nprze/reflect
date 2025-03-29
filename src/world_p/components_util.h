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
        case ComponentEnum::cameraComponent: {\
            auto* vec = static_cast<std::vector<cameraComponent>*>(compVecPtr);\
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

#define RFCT_ADD_SINGLE_COMPONENT(type, newArchetype, oldArchetype, entity)                                                                                                      \
    switch (type) {                                                                                                                                                     \
        case ComponentEnum::nameComponent: {                                                                                                                          \
            newArchetype->addsingleComponent<nameComponent>(oldArchetype->getComponent<nameComponent>(m_EntityLocations[entity].locationIndex));                      \
            break;                                                                                                                                                    \
        }                                                                                                                                                             \
        case ComponentEnum::damageComponent: {                                                                                                                        \
            newArchetype->addsingleComponent<damageComponent>(oldArchetype->getComponent<damageComponent>(m_EntityLocations[entity].locationIndex));                  \
            break;                                                                                                                                                    \
        }                                                                                                                                                             \
        case ComponentEnum::healthComponent: {                                                                                                                        \
            newArchetype->addsingleComponent<healthComponent>(oldArchetype->getComponent<healthComponent>(m_EntityLocations[entity].locationIndex));                  \
            break;                                                                                                                                                    \
        }                                                                                                                                                             \
        case ComponentEnum::cameraComponent: {                                                                                                                        \
            newArchetype->addsingleComponent<cameraComponent>(oldArchetype->getComponent<cameraComponent>(m_EntityLocations[entity].locationIndex));                  \
            break;                                                                                                                                                    \
        }                                                                                                                                                             \
        default: {                                                                                                                                                    \
            RFCT_CRITICAL("Couldn't add entity components");                                                                                                          \
            break;                                                                                                                                                    \
        }                                                                                                                                                             \
    }

#define RFCT_ADD_COMPONENT(type, archetype)                                                                         \
    switch (type) {                                                                                                     \
    case ComponentEnum::nameComponent: {                                                                                \
        archetype->addComponent<nameComponent>();                                                                       \
        break;                                                                                                          \
    }                                                                                                                   \
    case ComponentEnum::damageComponent: {                                                                              \
        archetype->addComponent<damageComponent>();                                                                     \
        break;                                                                                                          \
    }                                                                                                                   \
    case ComponentEnum::healthComponent: {                                                                              \
        archetype->addComponent<healthComponent>();                                                                     \
        break;                                                                                                          \
    }                                                                                                                   \
    case ComponentEnum::cameraComponent: {                                                                              \
        archetype->addComponent<cameraComponent>();                                                                     \
        break;                                                                                                          \
    }                                                                                                                   \
    default:                                                                                                            \
        RFCT_CRITICAL("Couldn't add entity components");                                                                \
        break;                                                                                                          \
    }
