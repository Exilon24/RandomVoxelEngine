#include <shader.hpp>

Shader::Shader(ShaderType type) : shaderType(type) {}
Shader::Shader(const char* vertex, const char* fragment)
{
    shaderType = ShaderType::Render;
    SetVertex(vertex);
    SetFragment(fragment);
    CompileShader();
}
Shader::Shader(const char* compute)
{
    shaderType = ShaderType::Compute;
    SetCompute(compute);
    CompileShader();
}


std::string Shader::readFile(const char* path)
{
    std::string code;
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        code = stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR: SHADER FILE COULD NOT BE READ: " << e.what() << "\n";
    }

    return code;
}

void Shader::SetFragment(const char* path)
{
    if (shaderType != ShaderType::Render)
    {
        std::cerr << "This is not a renderer shader!\n";
        return;
    }

    std::string code = readFile(path);
    const char* c_strCode = code.c_str();
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &c_strCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
}

void Shader::SetVertex(const char* path)
{
    if (shaderType != ShaderType::Render)
    {
        std::cerr << "This is not a renderer shader!\n";
        return;
    }

    std::string code = readFile(path);
    const char* c_strCode = code.c_str();
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &c_strCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
}

void Shader::SetCompute(const char* path)
{
    if (shaderType != ShaderType::Compute)
    {
        std::cerr << "This is not a compute shader!\n";
        return;
    }

    std::string code = readFile(path);
    const char* c_strCode = code.c_str();
    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &c_strCode, NULL);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE");
}

void Shader::CompileShader()
{
    // shader Program
    ID = glCreateProgram();
    if (shaderType == ShaderType::Render)
    {
        std::cout << "Compiling render shader...\n";
        std::cout << "Fragment: " << fragment << "\nVertex: " << vertex << "\ncompute: " << compute << "\n";
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);

        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    else if (shaderType == ShaderType::Compute)
    {
        std::cout << "Compiling compute shader...\n";
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(compute);
    }
}

// activate the shader
// ------------------------------------------------------------------------
void Shader::use() const
{
    glUseProgram(ID);
}
// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
