/*
The MIT License (MIT)

Copyright © 2024 Ivan Reshetnikov

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the “Software”), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define DEBUG
#define STB_IMAGE_IMPLEMENTATION
#include "pch.h"
#include "camera.h"

Camera camera;

struct MeshSegment
{
    std::string name;
    GLuint diffuseTextureID = 0;
    int vertexBufferStartIndex = 0;
    int vertexBufferDuration = 0;
};

struct Mesh
{
    std::vector<MeshSegment> meshSegments;
    std::vector<float> vertices;
};

std::string readFile(const char* path);

GLuint createShaderProgram(const char *vertexShaderSource, const char *fragmentShaderSource);

Mesh loadOBJ(const char* path);

std::unordered_map<std::string, MeshSegment> loadMaterialsMTLLIB(const char* path);

int main(void)
{
    // Setup
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(640, 480, "OpenGL", glfwGetPrimaryMonitor(), NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD!\n");
        return -1;
    }

    glViewport(0, 0, 640, 480);

    // Mesh
    Mesh mesh = loadOBJ("doom.obj");

    GLuint VBO, VAO;
    // GLuint EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Shader
    GLuint shaderProgram = createShaderProgram(readFile("vertex.glvs").c_str(), readFile("fragment.glfs").c_str());

    // Camera setup
    camera.setAspectRatio(640, 480);
    camera.farPlane = 256.0f;
    camera.position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::mat4 projMatrix = camera.getProjectionMatrix();

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!glfwWindowShouldClose(window))
    {
        camera.position.x = (float)sin((float)glfwGetTime() * 0.2f) * 11.0f;
        camera.position.z = (float)cos((float)glfwGetTime() * 0.2f) * 11.0f;
        camera.position.y = 2.0f;
        
        glm::vec3 center(0.0f, 1.5f, 0.0f); // Center of the scene
        glm::vec3 direction = glm::normalize(center - camera.position);

        // Calculate yaw (rotation around Y axis)
        camera.yaw = glm::degrees(atan2(direction.z, direction.x));

        // Calculate pitch (rotation around X axis)
        camera.pitch = glm::degrees(asin(direction.y));

        // Set the new camera direction
        camera.front = glm::normalize(direction);

        glm::mat4 viewMatrix = camera.getViewMatrix();

        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        glUseProgram(shaderProgram);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glBindVertexArray(VAO);
        for (const auto& materialSegment : mesh.meshSegments)
        {
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
            glBindTexture(GL_TEXTURE_2D, materialSegment.diffuseTextureID);

            glDrawArrays(GL_TRIANGLES, materialSegment.vertexBufferStartIndex, materialSegment.vertexBufferDuration);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto& materialSegment : mesh.meshSegments)
        glDeleteTextures(1, &materialSegment.diffuseTextureID);
    
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}


GLuint createShaderProgram(const char *vertexShaderSource, const char *fragmentShaderSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
#ifdef DEBUG
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
#endif

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
#ifdef DEBUG
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
#endif
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
#ifdef DEBUG
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
#endif

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


std::string readFile(const char* path) {
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

Mesh loadOBJ(const char *path)
{
#ifdef DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    std::vector<glm::vec3> vertexPositions;
    std::vector<glm::vec2> vertexUVs;
    std::vector<glm::vec3> vertexNormals;
    bool loadedMaterial = false;
    int facesAdded = 0;
    int facesAddedSinceLastMaterial = 0;
    std::string lastMaterialName;

    std::unordered_map<std::string, MeshSegment> meshSegments;
    std::vector<float> vertices;

    std::ifstream file(path, std::ios::in | std::ios::binary);
        if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream lineStream(line);
            std::string identifier;
            lineStream >> identifier;

            if (identifier == "v") {
                glm::vec3 vertexPosition;
                lineStream >> vertexPosition.x >> vertexPosition.y >> vertexPosition.z;
                vertexPositions.push_back(vertexPosition);
            } else if (identifier == "vt") {
                glm::vec2 vertexUV;
                lineStream >> vertexUV.x >> vertexUV.y;
                vertexUVs.push_back(vertexUV);
            } else if (identifier == "vn") {
                glm::vec3 vertexNormal;
                lineStream >> vertexNormal.x >> vertexNormal.y >> vertexNormal.z;
                vertexNormals.push_back(vertexNormal);
            } else if (identifier == "f") {
                char slash;  // Temporary variable to skip "/"
#pragma loop(unroll)
                for (int i = 0; i < 3; ++i) {
                    int vIndex, tIndex, nIndex;
                    lineStream >> vIndex >> slash >> tIndex >> slash >> nIndex;

                    const glm::vec3& vertexPos = vertexPositions[vIndex - 1];
                    vertices.push_back(vertexPos.x);
                    vertices.push_back(vertexPos.y);
                    vertices.push_back(vertexPos.z);

                    const glm::vec2& vertexUV = vertexUVs[tIndex - 1];
                    vertices.push_back(vertexUV.x);
                    vertices.push_back(vertexUV.y);

                    const glm::vec3& vertexNormal = vertexNormals[nIndex - 1];
                    vertices.push_back(vertexNormal.x);
                    vertices.push_back(vertexNormal.y);
                    vertices.push_back(vertexNormal.z);
                }
                facesAdded += 3;
            
            } else if (identifier == "mtllib") {
                if (loadedMaterial) { continue; }

                std::string materialFileName;
                lineStream >> materialFileName;

                std::filesystem::path modelPath(path);
                std::string materialFilePath = (modelPath.parent_path() / materialFileName).string();

                meshSegments = loadMaterialsMTLLIB(materialFilePath.c_str());
                loadedMaterial = true;
            
            } else if (identifier == "usemtl") {
                std::string materialName;
                lineStream >> materialName;

                meshSegments[materialName].vertexBufferStartIndex = facesAdded;
                
                if (!lastMaterialName.empty())
                    meshSegments[lastMaterialName].vertexBufferDuration = facesAdded - facesAddedSinceLastMaterial;

                facesAddedSinceLastMaterial = facesAdded;
                lastMaterialName = materialName;
            }
        }
        file.close();
    }

#ifdef DEBUG
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
#endif

    Mesh mesh;
    mesh.vertices = vertices;

    for (const auto& entry : meshSegments) {
        mesh.meshSegments.push_back(entry.second);
    }

    return mesh;
}

std::unordered_map<std::string, MeshSegment> loadMaterialsMTLLIB(const char* path)
{
    std::string currentMaterialKey;
    std::unordered_map<std::string, MeshSegment> meshSegments;

    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream lineStream(line);
            std::string identifier;
            lineStream >> identifier;

            if (identifier == "newmtl") {
                std::string materialName;
                lineStream >> materialName;

                MeshSegment materialSegment;
                materialSegment.name = materialName;
                meshSegments[materialName] = materialSegment;
                
                currentMaterialKey = materialName;

            } else if (identifier == "map_Kd") {
                if (currentMaterialKey.empty()) continue;
                std::string diffuseMapPath;
                lineStream >> diffuseMapPath;
                
                MeshSegment& currentMaterialSegment = meshSegments[currentMaterialKey];

                GLuint textureID = 0;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                stbi_set_flip_vertically_on_load(1);
                int textureWidth, textureHeight, textureChannelCount;
                unsigned char* data = stbi_load(diffuseMapPath.c_str(), &textureWidth, &textureHeight, &textureChannelCount, 0);
                if (data) {
                    glTexImage2D(GL_TEXTURE_2D, 0, textureChannelCount == 4 ? GL_RGBA : GL_RGB, textureWidth, textureHeight, 0, textureChannelCount == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
                    currentMaterialSegment.diffuseTextureID = textureID;
                } else {
                    std::cout << "ERROR::TEXTURE::LOAD_FAILED" << std::endl;
                    glDeleteTextures(1, &textureID);
                }
                stbi_image_free(data);
            }
        }
        file.close();
    }

    return meshSegments;
}