﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "Shader_OpenGL.h"

#include <fstream>
#include <GL/glew.h>

namespace JumaRenderEngine
{
    jarray<jstring> LoadOpenGLShaderFile(const jstring& fileName, const bool shouldLogErrors)
    {
        std::ifstream file(*fileName);
        if (!file.is_open())
        {
            if (shouldLogErrors)
            {
                JUTILS_LOG(error, JSTR("Failed to open file {}"), fileName);
            }
            return {};
        }

        jarray<jstring> result;
        while (!file.eof())
        {
            std::string line;
            std::getline(file, line);
            line += '\n';
            result.add(jstring(line));
        }
        file.close();
        return result;
    }
    uint32 LoadOpenGLShader_Text(const jstring& fileName, const GLenum shaderStage, const bool optionalShader)
    {
        const jarray<jstring> shaderText = LoadOpenGLShaderFile(fileName, !optionalShader);
        if (shaderText.isEmpty())
        {
            if (!optionalShader)
            {
                JUTILS_LOG(error, JSTR("Failed to load shader file {}"), fileName);
            }
            return 0;
        }
        jarray<const GLchar*> shaderLines(shaderText.getSize());
        jarray<GLint> shaderLineLength(shaderText.getSize());
        for (int32 lineIndex = 0; lineIndex < shaderText.getSize(); lineIndex++)
        {
            const jstring& line = shaderText[lineIndex];
            shaderLines[lineIndex] = *line;
            shaderLineLength[lineIndex] = static_cast<GLint>(line.getSize());
        }

        const uint32 shaderIndex = glCreateShader(shaderStage);
        glShaderSource(shaderIndex, shaderLines.getSize(), shaderLines.getData(), shaderLineLength.getData());
        return shaderIndex;
    }

    jarray<int8> LoadOpenGLBinShaderFile(const jstring& fileName, const bool shouldLogErrors)
    {
        std::ifstream file(*fileName, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            if (shouldLogErrors)
            {
                JUTILS_LOG(error, JSTR("Failed to open file {}"), fileName);
            }
            return {};
        }

        file.seekg(0, std::ios::end);
        jarray<int8> result(static_cast<int32>(file.tellg()), 0);
        if (!result.isEmpty())
        {
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(result.getData()), result.getSize());
        }
        file.close();
        return result;
    }
    uint32 LoadOpenGLShader_Binary(const jstring& fileName, const GLenum shaderStage, const bool optionalShader)
    {
        const jarray<int8> shaderData = LoadOpenGLBinShaderFile(fileName, !optionalShader);
        if (shaderData.isEmpty())
        {
            if (!optionalShader)
            {
                JUTILS_LOG(error, JSTR("Failed to load shader file {}"), fileName);
            }
            return 0;
        }

        const uint32 shaderIndex = glCreateShader(shaderStage);
        glShaderBinary(1, &shaderIndex, GL_SHADER_BINARY_FORMAT_SPIR_V, shaderData.getData(), shaderData.getSize());
        glSpecializeShader(shaderIndex, "main", 0, nullptr, nullptr);
        return shaderIndex;
    }

    uint32 CompileOpenGLShader(const bool binary, const jstring& fileName, const GLenum shaderStage, const bool optionalShader = false)
    {
        const uint32 shaderIndex = binary ? LoadOpenGLShader_Binary(fileName, shaderStage, optionalShader) : LoadOpenGLShader_Text(fileName, shaderStage, optionalShader);
        if (shaderIndex == 0)
        {
            return 0;
        }
        if (!binary)
        {
            glCompileShader(shaderIndex);
        }

        GLint compileStatus;
        glGetShaderiv(shaderIndex, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus == GL_FALSE)
        {
#ifndef JUTILS_LOG_DISABLED
            GLint logLength;
            glGetShaderiv(shaderIndex, GL_INFO_LOG_LENGTH, &logLength);
            jstring message(logLength, ' ');
            glGetShaderInfoLog(shaderIndex, logLength, &logLength, *message);
            JUTILS_LOG(error, JSTR("Failed to compile shader {}: {}"), fileName, message);
#endif
            glDeleteShader(shaderIndex);
            return 0;
        }
        return shaderIndex;
    }
    bool CompileOpenGLShader(uint32& outShaderIndex, const jmap<ShaderStageFlags, jstring>& fileNames, const ShaderStageFlags shaderStage, 
        const GLenum shaderStageOpenGL, const bool optionalShader = false)
    {
        constexpr bool binary = false;
        const jstring* fileNamePtr = fileNames.find(shaderStage);
        if (fileNamePtr != nullptr)
        {
            const uint32 shader = CompileOpenGLShader(binary, *fileNamePtr, shaderStageOpenGL, optionalShader);
            if (shader != 0)
            {
                outShaderIndex = shader;
                return true;
            }
        }
        if (optionalShader)
        {
            outShaderIndex = 0;
            return true;
        }
        return false;
    }

    Shader_OpenGL::~Shader_OpenGL()
    {
        clearOpenGL();
    }

    bool Shader_OpenGL::initInternal(const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        bool success = true;

        constexpr uint8 shadersCount = 2;
        uint32 shaderIndices[shadersCount] = { 0, 0 };
        if (!CompileOpenGLShader(shaderIndices[0], fileNames, SHADER_STAGE_VERTEX, GL_VERTEX_SHADER, false) ||
            !CompileOpenGLShader(shaderIndices[1], fileNames, SHADER_STAGE_FRAGMENT, GL_FRAGMENT_SHADER, false))
        {
            JUTILS_LOG(error, JSTR("Failed to load shader"));
            success = false;
        }
        else
        {
            const uint32 shaderProgramIndex = glCreateProgram();
            for (const uint32 shaderIndex : shaderIndices)
            {
                if (shaderIndex != 0)
                {
                    glAttachShader(shaderProgramIndex, shaderIndex);
                }
            }
            glLinkProgram(shaderProgramIndex);

            GLint linkStatus;
            glGetProgramiv(shaderProgramIndex, GL_LINK_STATUS, &linkStatus);
            if (linkStatus == GL_FALSE)
            {
#ifndef JUTILS_LOG_DISABLED
                int logLength;
                glGetProgramiv(shaderProgramIndex, GL_INFO_LOG_LENGTH, &logLength);

                jstring message(logLength, ' ');
                glGetProgramInfoLog(shaderProgramIndex, logLength, &logLength, *message);
                JUTILS_LOG(error, JSTR("Failed to compile shader program: {}"), message);
#endif
                glDeleteProgram(shaderProgramIndex);
                success = false;
            }
            else
            {
                m_ShaderProgramIndex = shaderProgramIndex;
            }
        }
        for (const uint32 shaderIndex : shaderIndices)
        {
            if (shaderIndex != 0)
            {
                glDeleteShader(shaderIndex);
            }
        }

        return success;
    }

    void Shader_OpenGL::onClearAsset()
    {
        clearOpenGL();
        Super::onClearAsset();
    }
    void Shader_OpenGL::clearOpenGL()
    {
        if (m_ShaderProgramIndex != 0)
        {
            glDeleteProgram(m_ShaderProgramIndex);
            m_ShaderProgramIndex = 0;
        }
    }

    bool Shader_OpenGL::activateShader() const
    {
        if (m_ShaderProgramIndex != 0)
        {
            glUseProgram(m_ShaderProgramIndex);
            return true;
        }
        return false;
    }
    void Shader_OpenGL::deactivateAnyShader()
    {
        glUseProgram(0);
    }
}

#endif
