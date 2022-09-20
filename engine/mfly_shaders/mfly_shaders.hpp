#ifndef MFLY_SHADERS
#define MFLY_SHADERS

#include <stdint.h>
#include <shaderc/shaderc.hpp>


namespace mfly
{
    namespace shaders
    {
        enum ShaderKind {
            INFER = shaderc_glsl_infer_from_source,
            
            VERTEX = shaderc_vertex_shader,
            FRAGMENT = shaderc_fragment_shader,
            COMPUTE = shaderc_compute_shader,
            GEOMTERY = shaderc_geometry_shader,
            TESS_CTRL = shaderc_tess_control_shader,
            TESS_EVAL = shaderc_tess_evaluation_shader,

            //...
        };

        // Module Functions
        uint16_t Init();
        uint16_t Close();
        
        struct Shader {
            const char* code;
            uint32_t len;
            ShaderKind type;
        };
        uint16_t AddShader(const char* code, ShaderKind type);
        uint16_t AddShaders(Shader* shaders, uint64_t num_shaders);
        uint16_t FlushShaders();

        uint16_t ObtainFiles(); // Receive uncompiled shaders
        uint16_t AddDefines(const char* def, const char* val);
        uint16_t CreateGroup(uint16_t* shader_handles, uint64_t num_shaders, uint16_t* define_handles, uint64_t num_defines);

        // Generate Defines
        // Genearate Groups to be compiled with single options and defines
        
        uint16_t CompileGroups(bool optimization = false, int opt_level = 3);
        uint16_t CompileSpecific(uint16_t* group_handles, uint64_t num_groups, bool optimization = true); // Compile Shaders and return SPIR-V bytecode representation
        uint16_t FlushShaders();

        struct ShaderByteCode {
            std::vector<uint32_t> bytecode;
        };
        // Requires initialized space, it will be moved in
        uint64_t ReturnBytecode(std::vector<ShaderByteCode>& fill); // Return compiled shaders to tool that actually uses them

        enum ERRORCODE
        {
            GOOD = 0,
            BAD = 1,
        };
#if defined(DEBUG_STRINGS)
        const char*[] debug_strings
        {
            "Good, move on.",
            "Something went wrong, message not set",
        }
        const char* DebugMessage(uint16_t errorcode) {
            if(errorcode > (sizeof(debug_string)/sizeof(const char*)))
                return debug_string[1];
            else
                return debug_string[errorcode];
        }
#endif

        // Module Specifics
        
    };
};

#endif