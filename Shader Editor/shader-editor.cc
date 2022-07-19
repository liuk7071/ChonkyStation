/***************************************************************************
 *   Copyright (C) 2021 PCSX-Redux authors                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "shader-editor.h"

#include <filesystem>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <streambuf>

#include <glad/glad.h>

static const GLchar* const c_defaultVertexShader = R"(
#version 430 core
// The Vertex Shader isn't necessarily very useful, but is still provided here.
precision highp float;
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;
uniform mat4 u_projMatrix;
out vec2 Frag_UV;
out vec4 Frag_Color;
void main() {
    Frag_UV = UV;
    Frag_Color = Color;
    gl_Position = u_projMatrix * vec4(Position.xy, 0, 1);
}
)";

static const GLchar* const c_defaultPixelShader = R"(
#version 430 core
// The Pixel Shader is most likely what the user will want to change.
precision highp float;
uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Color;
layout (location = 0) out vec4 Out_Color;
void main() {
    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
    Out_Color.a = 1.0;
}
)";

static const char* const c_defaultLuaInvoker = R"(
-- All of this code is sandboxed, as in any global variable it
-- creates will be attached to a local environment. The global
-- environment is still accessible as normal.
-- Note that the environment is wiped every time the shader is
-- recompiled one way or another.

-- The environment will have the `shaderProgramID` variable set
-- before being evaluated, meaning this code is perfectly valid:
function Constructor(shaderProgramID)
    -- Cache some Shader Attributes locations
    print('Shader compiled: program ID = ' .. shaderProgramID)
end
Constructor(shaderProgramID)

-- This function is called to issue an ImGui::Image when it's time
-- to display the video output of the emulated screen. It can
-- prepare some values to later attach to the shader program.
--
-- This function won't be called for non-ImGui renders, such as
-- the offscreen render of the vram.
function Image(textureID, srcSizeX, srcSizeY, dstSizeX, dstSizeY)
    imgui.Image(textureID, dstSizeX, dstSizeY, 0, 0, 1, 1)
end

-- This function is called to draw some UI, at the same time
-- as the shader editor, but regardless of the status of the
-- shader editor window. Its purpose is to potentially display
-- a piece of UI to let the user interact with the shader program.

-- Returning true from it will cause the environment to be saved.
function Draw()
end

-- This function is called just before executing the shader program,
-- to give it a chance to bind some attributes to it, that'd come
-- from either the global state, or the locally computed attributes
-- from the two functions above.
--
-- The last six parameters will only exist for non-ImGui renders.
function BindAttributes(textureID, shaderProgramID, srcLocX, srcLocY, srcSizeX, srcSizeY, dstSizeX, dstSizeY)
end
)";

ShaderEditor::ShaderEditor(const std::string& base, const std::string_view& dVS,
    const std::string_view& dPS, const std::string_view& dL)
    : m_baseFilename(base) {
    std::filesystem::path f = base;
    {
        f.replace_extension("vert");
        std::ifstream in(f, std::ifstream::in);
        if (in) {
            std::ostringstream code;
            code << in.rdbuf();
            in.close();
            m_vertexShaderEditor.setText(code.str());
        }
        else {
            m_vertexShaderEditor.setText(c_defaultVertexShader);
        }
    }
    {
        f.replace_extension("frag");
        std::ifstream in(f, std::ifstream::in);
        if (in) {
            std::ostringstream code;
            code << in.rdbuf();
            in.close();
            m_pixelShaderEditor.setText(code.str());
        }
        else {
            m_pixelShaderEditor.setText(c_defaultPixelShader);
        }
    }
    {
        f.replace_extension("lua");
        std::ifstream in(f, std::ifstream::in);
        if (in) {
            std::ostringstream code;
            code << in.rdbuf();
            in.close();
            m_luaEditor.setText(code.str());
        }
        else {
            m_luaEditor.setText(c_defaultLuaInvoker);
        }
    }
}

ShaderEditor::~ShaderEditor() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
    }
}

void ShaderEditor::setDefaults() {
    m_vertexShaderEditor.setText(c_defaultVertexShader);
    m_pixelShaderEditor.setText(c_defaultPixelShader);
    m_luaEditor.setText(c_defaultLuaInvoker);
}

void ShaderEditor::init() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
}

std::optional<GLuint> ShaderEditor::compile(
    const std::vector<std::string_view>& mandatoryAttributes) {
    m_setupVAO = true;
    m_shaderProjMtxLoc = -1;
    GLint status = 0;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    auto VS = getVertexText();
    const char* VSv = VS.data();
    glShaderSource(vertexShader, 1, &VSv, 0);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        GLint maxLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
        char* log = (char*)malloc(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, log);

        m_errorMessage = format("Vertex Shader compilation error: %s\n", log);

        free(log);
        glDeleteShader(vertexShader);
        return std::nullopt;
    }

    GLuint pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
    auto PS = getPixelText();
    const char* PSv = PS.data();
    glShaderSource(pixelShader, 1, &PSv, 0);
    glCompileShader(pixelShader);

    glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        GLint maxLength;
        glGetShaderiv(pixelShader, GL_INFO_LOG_LENGTH, &maxLength);
        char* log = (char*)malloc(maxLength);

        glGetShaderInfoLog(pixelShader, maxLength, &maxLength, log);

        m_errorMessage = format("Pixel Shader compilation error: %s\n", log);

        free(log);
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);
        return std::nullopt;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, pixelShader);

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == 0) {
        GLint maxLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        char* log = (char*)malloc(maxLength);

        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, log);

        m_errorMessage = format("Link error: %s\n", log);

        free(log);
        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(pixelShader);
        return std::nullopt;
    }

    for (auto attrib : mandatoryAttributes) {
        int loc = glGetAttribLocation(shaderProgram, attrib.data());
        if (loc == -1) {
            m_errorMessage = format(("Missing attribute %s in shader program"), attrib);
            glDeleteProgram(shaderProgram);
            glDeleteShader(vertexShader);
            glDeleteShader(pixelShader);
            return std::nullopt;
        }
    }

    glDeleteShader(vertexShader);
    glDeleteShader(pixelShader);

    m_errorMessage.clear();

    std::filesystem::path f = m_baseFilename;

    if (m_autosave) {
        {
            f.replace_extension("vert");
            std::ofstream out(f, std::ofstream::out);
            out << m_vertexShaderEditor.getText();
        }
        {
            f.replace_extension("frag");
            std::ofstream out(f, std::ofstream::out);
            out << m_pixelShaderEditor.getText();
        }
        {
            f.replace_extension("lua");
            std::ofstream out(f, std::ofstream::out);
            out << m_luaEditor.getText();
        }
    }

    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
    }
    m_shaderProgram = shaderProgram;
    m_shaderProjMtxLoc = glGetUniformLocation(m_shaderProgram, "u_projMatrix");
    return shaderProgram;
}

bool ShaderEditor::draw(const char* title) {
    if (!m_show) return false;
    if (!ImGui::Begin(title, &m_show)) return false;
    ImGui::Checkbox("Auto reload", &m_autoreload);
    ImGui::SameLine();
    ImGui::Checkbox("Auto save", &m_autosave);
    ImGui::SameLine();
    ImGui::Checkbox("Show all", &m_showAll);
    ImGui::SameLine();
    if (ImGui::Button("Configure shader")) {
        setConfigure();
    }
    auto contents = ImGui::GetContentRegionAvail();
    ImGuiStyle& style = ImGui::GetStyle();
    const float heightSeparator = style.ItemSpacing.y;
    float footerHeight = heightSeparator * 2 + 5 * ImGui::GetTextLineHeightWithSpacing();
    float width = contents.x / 3 - style.ItemInnerSpacing.x;
    if (m_showAll) {
        ImVec2 size = { width, contents.y - footerHeight };
        ImGui::BeginChild("VertexShaderEditor", size);
        m_vertexShaderEditor.draw();
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("PixelShaderEditor", size);
        m_pixelShaderEditor.draw();
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("LuaInvoker", size);
        m_luaEditor.draw();
        ImGui::EndChild();
    }
    else {
        if (ImGui::BeginTabBar("MyTabBar")) {
            ImVec2 size = { contents.x, contents.y - footerHeight };
            if (ImGui::BeginTabItem("Vertex Shader")) {
                ImGui::BeginChild("VertexShaderEditor", size);
                m_vertexShaderEditor.draw();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Pixel Shader")) {
                ImGui::BeginChild("PixelShaderEditor", size);
                m_pixelShaderEditor.draw();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    //ImGui::PopFont();
    ImGui::BeginChild("Errors", ImVec2(0, 0), true);
    ImGui::Text("%s", m_errorMessage.c_str());
    if (m_displayError) {
        for (auto& msg : m_lastLuaErrors) {
            ImGui::TextUnformatted(msg.c_str());
        }
    }
    ImGui::EndChild();

    ImGui::End();

    return m_vertexShaderEditor.hasTextChanged() || m_pixelShaderEditor.hasTextChanged() ||
        m_luaEditor.hasTextChanged();
}

void ShaderEditor::setConfigure(bool configure) {

}

void ShaderEditor::configure(GUI* gui) {
    
}

void ShaderEditor::reset(GUI* gui) {
    
}


void ShaderEditor::renderWithImgui(GUI* gui, ImTextureID textureID, const ImVec2& srcSize,
    const ImVec2& dstSize) {
    if (m_shaderProgram == 0) {
        compile();
    }
    if (m_shaderProgram == 0) {
        ImGui::Image(textureID, dstSize, { 0, 0 }, { 1, 1 });
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    m_cachedGui = gui;
    drawList->AddCallback(
        [](const ImDrawList* parentList, const ImDrawCmd* cmd) {
            ShaderEditor* that = reinterpret_cast<ShaderEditor*>(cmd->UserCallbackData);
            that->imguiCB(parentList, cmd);
        },
        this);

    
    drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void ShaderEditor::imguiCB(const ImDrawList* parentList, const ImDrawCmd* cmd) {
    GLuint textureID = static_cast<GLuint>(reinterpret_cast<uintptr_t>(cmd->TextureId));

    GLfloat projMtx[4][4];
    if (m_imguiProjMtxLoc == -1) {
        glGetIntegerv(GL_CURRENT_PROGRAM, &m_imguiProgram);
        m_imguiProjMtxLoc = glGetUniformLocation(m_imguiProgram, "ProjMtx");
    }

    // Get projection matrix from the Imgui program
    glUseProgram(m_shaderProgram);
    glGetUniformfv(m_imguiProgram, m_imguiProjMtxLoc, &projMtx[0][0]);

    // Send projection matrix to our shader
    glUniformMatrix4fv(m_shaderProjMtxLoc, 1, GL_FALSE, &projMtx[0][0]);
    //glBindTexture(GL_TEXTURE_2D, textureID);
}

void ShaderEditor::render(GUI* gui, GLuint textureID, const ImVec2& srcLoc, const ImVec2& srcSize,
    const ImVec2& dstSize) {
    if (m_shaderProgram == 0) {
        compile();
    }
    if (m_shaderProgram == 0) {
        return;
    }

    glBindVertexArray(m_vao);
    glUseProgram(m_shaderProgram);
    struct VertexData {
        float positions[3];
        float textures[2];
        float color[4];
    };

    VertexData quadVertices[4];

    quadVertices[0].positions[0] = -1.0;
    quadVertices[0].positions[1] = -1.0;
    quadVertices[0].positions[2] = 0.0;
    quadVertices[0].textures[0] = srcLoc.x;
    quadVertices[0].textures[1] = srcLoc.y;
    quadVertices[0].color[0] = 1.0;
    quadVertices[0].color[1] = 1.0;
    quadVertices[0].color[2] = 1.0;
    quadVertices[0].color[3] = 1.0;

    quadVertices[1].positions[0] = 1.0;
    quadVertices[1].positions[1] = -1.0;
    quadVertices[1].positions[2] = 0.0;
    quadVertices[1].textures[0] = srcLoc.x + srcSize.x;
    quadVertices[1].textures[1] = srcLoc.y;
    quadVertices[1].color[0] = 1.0;
    quadVertices[1].color[1] = 1.0;
    quadVertices[1].color[2] = 1.0;
    quadVertices[1].color[3] = 1.0;

    quadVertices[2].positions[0] = -1.0;
    quadVertices[2].positions[1] = 1.0;
    quadVertices[2].positions[2] = 0.0;
    quadVertices[2].textures[0] = srcLoc.x;
    quadVertices[2].textures[1] = srcLoc.y + srcSize.y;
    quadVertices[2].color[0] = 1.0;
    quadVertices[2].color[1] = 1.0;
    quadVertices[2].color[2] = 1.0;
    quadVertices[2].color[3] = 1.0;

    quadVertices[3].positions[0] = 1.0;
    quadVertices[3].positions[1] = 1.0;
    quadVertices[3].positions[2] = 0.0;
    quadVertices[3].textures[0] = srcLoc.x + srcSize.x;
    quadVertices[3].textures[1] = srcLoc.y + srcSize.y;
    quadVertices[3].color[0] = 1.0;
    quadVertices[3].color[1] = 1.0;
    quadVertices[3].color[2] = 1.0;
    quadVertices[3].color[3] = 1.0;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * 4, &quadVertices[0], GL_STATIC_DRAW);
    glDisable(GL_DEPTH_TEST);

    if (m_setupVAO) {
        m_setupVAO = false;
        int loc = glGetAttribLocation(m_shaderProgram, "Position");
        if (loc >= 0) {
            glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                (void*)&((VertexData*)nullptr)->positions);
            glEnableVertexAttribArray(loc);
        }

        loc = glGetAttribLocation(m_shaderProgram, "UV");
        if (loc >= 0) {
            glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                (void*)&((VertexData*)nullptr)->textures);
            glEnableVertexAttribArray(loc);
        }

        loc = glGetAttribLocation(m_shaderProgram, "Color");
        if (loc >= 0) {
            glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                (void*)&((VertexData*)nullptr)->color);
            glEnableVertexAttribArray(loc);
        }
    }

    GLfloat currentProjection[4][4];

    currentProjection[0][0] = 1.0f;
    currentProjection[0][1] = 0.0f;
    currentProjection[0][2] = 0.0f;
    currentProjection[0][3] = 0.0f;
    currentProjection[1][0] = 0.0f;
    currentProjection[1][1] = 1.0f;
    currentProjection[1][2] = 0.0f;
    currentProjection[1][3] = 0.0f;
    currentProjection[2][0] = 0.0f;
    currentProjection[2][1] = 0.0f;
    currentProjection[2][2] = 1.0f;
    currentProjection[2][3] = 0.0f;
    currentProjection[3][0] = 0.0f;
    currentProjection[3][1] = 0.0f;
    currentProjection[3][2] = 0.0f;
    currentProjection[3][3] = 1.0f;
    glUniformMatrix4fv(m_shaderProjMtxLoc, 1, GL_FALSE, &currentProjection[0][0]);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
