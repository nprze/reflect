import re

def extract_component_structs(header_content):
    pattern = re.compile(r'struct\s+(\w+Component)\s*{')
    return pattern.findall(header_content)

def generate_macros(component_names):
    remove_macro = """#define RFCT_REMOVE_COMPONENT_AT_INDEX(compVecPtr, type, index)\\\n    switch (type) {\\\n"""
    
    add_single_macro = """#define RFCT_ADD_SINGLE_COMPONENT(type, newArchetype, oldArchetype, entity)\\\n    switch (type) {\\\n"""
    
    add_macro = """#define RFCT_ADD_COMPONENT(type, archetype)\\\n    switch (type) {\\\n"""
    
    for component in component_names:
        remove_macro += f"        case ComponentEnum::{component}: {{\\\n" \
                        f"            auto* vec = static_cast<std::vector<{component}>*>(compVecPtr);\\\n" \
                        f"            if (index < vec->size() - 1) {{\\\n" \
                        f"                std::swap(vec->at(index), vec->back());\\\n" \
                        f"            }}\\\n" \
                        f"            vec->pop_back();\\\n" \
                        f"            break;\\\n" \
                        f"        }}\\\n"
        
        add_single_macro += f"        case ComponentEnum::{component}: {{\\\n" \
                            f"            newArchetype->addsingleComponent<{component}>(oldArchetype->getComponent<{component}>(m_EntityLocations[entity].locationIndex));\\\n" \
                            f"            break;\\\n" \
                            f"        }}\\\n"
        
        add_macro += f"        case ComponentEnum::{component}: {{\\\n" \
                     f"            archetype->addComponent<{component}>();\\\n" \
                     f"            break;\\\n" \
                     f"        }}\\\n"

    remove_macro += "        default:\\\n            RFCT_CRITICAL(\"Couldn't delete entity components\");\\\n            break;\\\n    }\n"
    add_single_macro += "        default: {\\\n            RFCT_CRITICAL(\"Couldn't add entity components\");\\\n            break;\\\n        }\\\n    }\n"
    add_macro += "        default:\\\n            RFCT_CRITICAL(\"Couldn't add entity components\");\\\n            break;\\\n    }\n"
    
    return remove_macro + "\n\n" + add_single_macro + "\n\n" + add_macro

def process_header_file(input_filename, output_filename):
    with open(input_filename, 'r') as file:
        content = file.read()
    
    component_names = extract_component_structs(content)
    macros = generate_macros(component_names)
    
    with open(output_filename, 'w') as file:
        file.write(macros)

if __name__ == "__main__":
    input_file = "components.h"
    output_file = "component_utils.h"
    process_header_file(input_file, output_file)
