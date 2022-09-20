#include "mfly_shaders.hpp"

namespace mfly {
    namespace shaders {
        
        std::vector<Shader> raw_shaders;


        // So fucking bad but should do for now
        // Start using shared pointers?
        struct Define {
            std::string name;
            std::string value;
        };
        std::vector<Define> defines;

        struct CompileGroup {
            std::vector<uint16_t> defines;
            std::vector<uint16_t> shaders;
        };
        std::vector<CompileGroup> groups;

        std::vector<ShaderByteCode> results;

        shaderc::Compiler compiler;

    };
};

uint16_t mfly::shaders::Init() {
    
    return 0;
}

uint16_t mfly::shaders::AddDefines(const char* def, const char* val) {
    defines.push_back(Define());
    Define& curr = defines.back();
    curr.name = std::string(def);
    curr.value = std::string(val);

    return defines.size()-1;
}

uint16_t mfly::shaders::AddShader(const char* code, ShaderKind type) {
    raw_shaders.push_back(Shader());
    Shader& s = raw_shaders.back();
    s.code = code;
    s.len = strlen(s.code);
    s.type = type;

    return raw_shaders.size()-1;
}

uint16_t mfly::shaders::AddShaders(Shader* shaders, uint64_t num_shaders) {
    uint16_t last_size = raw_shaders.size();
    raw_shaders.resize(raw_shaders.size() + num_shaders);
    memcpy(&raw_shaders[last_size], shaders, num_shaders * sizeof(Shader));

    return last_size; // return where shaders start, we already know the number
}

uint16_t mfly::shaders::FlushShaders() {
    // For now just delete all shaders and groups
    // Defines should not be cleaned

    raw_shaders.clear();
    groups.clear();

    return 0;
}

uint16_t mfly::shaders::CreateGroup(uint16_t* shader_handles, uint64_t num_shaders, uint16_t* define_handles, uint64_t num_defines) {
    groups.push_back(CompileGroup());
    CompileGroup& curr_group = groups.back();
    curr_group.defines.resize(num_defines);
    memcpy(curr_group.defines.data(), define_handles, sizeof(uint16_t)*num_defines);
    curr_group.shaders.resize(num_shaders);
    memcpy(curr_group.shaders.data(), shader_handles, sizeof(uint16_t)*num_shaders);
    
    return 0;
}

uint16_t mfly::shaders::CompileGroups(bool optimization, int opt_level) {
    for(auto shader_group : groups) {
        shaderc::CompileOptions options;
        const int s  = shader_group.defines.size();
        for(int i = 0; i < s; ++i) {
            const Define& def = defines[shader_group.defines[i]];
            options.AddMacroDefinition(def.name, def.value);
        }

        if(optimization)
            options.SetOptimizationLevel((shaderc_optimization_level)opt_level);
        
        for(auto shader : shader_group.shaders)
        {
            Shader& curr = raw_shaders[shader];
            shaderc::Compiler invoke;
            shaderc::SpvCompilationResult result = 
                invoke.CompileGlslToSpv(    curr.code, curr.len,
                                            (shaderc_shader_kind)(int)curr.type, 
                                            "src",
                                            options);

            // Handle Error Messages...
            if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
                printf("%s", result.GetErrorMessage().c_str());
            }
            else{
                //else worked
                results.push_back(ShaderByteCode());
                ShaderByteCode& curr = results.back();
                curr.bytecode = {result.cbegin(), result.cend()}; // Hate this initialization of a vector like this for a single shader...
            }
        }
    }

    FlushShaders();

    return 0;
}


uint64_t mfly::shaders::ReturnBytecode(std::vector<ShaderByteCode>& fill) {
    uint64_t num = results.size();
    results.swap(fill);
    results.clear();
    return num;
}