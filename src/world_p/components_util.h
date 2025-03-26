// please do not modify this file unless you know exactly what you are doing

#define RFCT_REMOVE_COMPONENT_AT_INDEX(pointer, type, index)\
        switch (type) { \
            case ComponentEnum::nameComponent:           \
                (static_cast<std::vector<nameComponent>*>(pointer))->at(index) = nameComponent();  \
            case ComponentEnum::damageComponent:         \
                static_cast<std::vector<damageComponent>*>(pointer)->at(index) = damageComponent(); \
            case ComponentEnum::healthComponent:         \
                static_cast<std::vector<healthComponent>*>(pointer)->at(index) = healthComponent(); \
            default:                                     \
                RFCT_CRITICAL("couldn't delete entity components");                        \
        }