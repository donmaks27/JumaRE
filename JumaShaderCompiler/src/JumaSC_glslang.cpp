// Copyright © 2023 Leonov Maksim. All rights reserved.

#include "CompilerInternal.h"

#ifdef JUMASC_ENABLE_GLSLANG

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <jutils/jdefer.h>

namespace JumaShaderCompiler
{
    bool InitializeGlslang(CompilerPart_glslang& outData)
    {
        if (!outData.initialized)
        {
            if (!glslang::InitializeProcess())
            {
                JUTILS_LOG(error, "failed to initialize glslang: glslang::InitializeProcess returned false");
                return false;
            }
            outData.initialized = true;
        }
        return true;
    }
    CompilerPart_glslang::~CompilerPart_glslang()
    {
        if (initialized)
        {
            initialized = false;
            glslang::FinalizeProcess();
        }
    }

    jarray<uint32> CompilerInternal::glslToSPV(const jarray<jstring>& shaderText, const GLSL::type shaderType,
        const Vulkan::version vulkanVersion)
    {
        if (!InitializeGlslang(glslangPart))
        {
            JUTILS_LOG(error, "failed to initialize glslang");
            return {};
        }

        EShLanguage glslangShaderType;
        glslang::EShTargetClientVersion glslangClientVersion;
        switch (shaderType)
        {
        case GLSL::type::vertex:          glslangShaderType = EShLanguage::EShLangVertex; break;
        case GLSL::type::fragment:        glslangShaderType = EShLanguage::EShLangFragment; break;
        case GLSL::type::geometry:        glslangShaderType = EShLanguage::EShLangGeometry; break;
        case GLSL::type::tess_control:    glslangShaderType = EShLanguage::EShLangTessControl; break;
        case GLSL::type::tess_evaluation: glslangShaderType = EShLanguage::EShLangTessEvaluation; break;
        case GLSL::type::compute:         glslangShaderType = EShLanguage::EShLangCompute; break;
        default:
            JUTILS_LOG(error, "invalid shader glslangShaderType");
            return {};
        }
        switch (vulkanVersion)
        {
        case Vulkan::version::_1_0: glslangClientVersion = glslang::EshTargetClientVersion::EShTargetVulkan_1_0; break;
        case Vulkan::version::_1_1: glslangClientVersion = glslang::EshTargetClientVersion::EShTargetVulkan_1_1; break;
        case Vulkan::version::_1_2: glslangClientVersion = glslang::EshTargetClientVersion::EShTargetVulkan_1_2; break;
        case Vulkan::version::_1_3: glslangClientVersion = glslang::EshTargetClientVersion::EShTargetVulkan_1_3; break;
        default:
            JUTILS_LOG(error, "invalid Vulkan version");
            return {};
        }
        glslang::TShader* shader = new glslang::TShader(glslangShaderType);
        jdefer deleteShader([shader](){ delete shader; });
        shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
        shader->setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetVulkan_1_3);

        jarray<const char*> glslangShaderText;
        glslangShaderText.reserve(shaderText.getSize());
        for (const auto& shaderTextLine : shaderText)
        {
            glslangShaderText.add(shaderTextLine.getData());
        }
        shader->setStrings(glslangShaderText.getData(), glslangShaderText.getSize());

        std::string preprocessedShaderText;
        glslang::TShader::ForbidIncluder glslangIncluder{};
        bool result = shader->preprocess(
            GetDefaultResources(), 100, EProfile::ECoreProfile,
            false, false, EShMessages::EShMsgDefault,
            &preprocessedShaderText, glslangIncluder
        );
        if (!result)
        {
            JUTILS_LOG(error, "failed to preprocess GLSL shader:\n{}\n{}", shader->getInfoLog(), shader->getInfoDebugLog());
            //printShaderText(shaderText);
            return {};
        }
        const char* preprocessedShaderTextStr = preprocessedShaderText.c_str();
        shader->setStrings(&preprocessedShaderTextStr, 1);

        result = shader->parse(
            GetDefaultResources(), 100, EProfile::ECoreProfile,
            false, false, EShMessages::EShMsgDefault
        );
        if (!result)
        {
            JUTILS_LOG(error, "failed to parse GLSL shader:\n{}\n{}", shader->getInfoLog(), shader->getInfoDebugLog());
            //printShaderText(shaderText);
            return {};
        }

        glslang::TProgram* program = new glslang::TProgram();
        jdefer deleteProgram([program](){ delete program; });
        program->addShader(shader);
        result = program->link(static_cast<EShMessages>(EShMessages::EShMsgSpvRules | EShMessages::EShMsgVulkanRules));
        if (!result)
        {
            JUTILS_LOG(error, "failed to link GLSL shader:\n{}\n{}", program->getInfoLog(), program->getInfoDebugLog());
            //printShaderText(shaderText);
            return {};
        }

        spv::SpvBuildLogger spvLogger;
        std::vector<unsigned int> spvShader;
        glslang::SpvOptions spvOptions{};
        spvOptions.generateDebugInfo = false;
        spvOptions.stripDebugInfo = false;
        spvOptions.emitNonSemanticShaderDebugInfo = false;
        spvOptions.emitNonSemanticShaderDebugSource = false;
        spvOptions.disableOptimizer = true;
        spvOptions.optimizeSize = false;
        spvOptions.disassemble = false;
        spvOptions.validate = true;
        glslang::GlslangToSpv(
            *program->getIntermediate(shader->getStage()),
            spvShader, &spvLogger, &spvOptions
        );
        const jstring spvMessages = spvLogger.getAllMessages();
        if (!spvMessages.isEmpty())
        {
            JUTILS_LOG(warning, "{}", spvMessages);
        }
        return spvShader;
    }
}

#else

namespace JumaShaderCompiler
{
    jarray<uint32> CompilerInternal::glslToSPV(const jarray<jstring>&, GLSL::type, Vulkan::version)
    {
        JUTILS_LOG(warning, "compiling GLSL shaders is disabled");
        return {};
    }
}

#endif