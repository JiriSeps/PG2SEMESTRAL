#include <string>
#include <GL/glew.h> 
#include <glm/glm.hpp>

#include "OBJloader.h"

#define MAX_LINE_SIZE 255

bool loadOBJ(const char* path, std::vector < glm::vec3 >& out_vertices,
    std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals)
{
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;

    out_vertices.clear();
    out_uvs.clear();
    out_normals.clear();

    FILE* file;
    fopen_s(&file, path, "r");
    if (file == NULL) {
        printf("Impossible to open the file !\n");
        return false;
    }

    while (true) {
        char lineHeader[128];
        int res = fscanf_s(file, "%s", lineHeader, sizeof(lineHeader));
        if (res == EOF) break;

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            char vertexStr[128];

            unsigned int vIndex[3], vtIndex[3], vnIndex[3];
            bool hasUV = false, hasNormal = false;

            for (int i = 0; i < 3; i++) {
                fscanf_s(file, "%s", vertexStr, (unsigned)_countof(vertexStr));
                std::string vert(vertexStr);

                size_t firstSlash = vert.find('/');
                size_t secondSlash = vert.find('/', firstSlash + 1);

                if (firstSlash == std::string::npos) {
                    // Format: v
                    vIndex[i] = std::stoi(vert);
                }
                else if (secondSlash == std::string::npos) {
                    // Format: v/vt
                    vIndex[i] = std::stoi(vert.substr(0, firstSlash));
                    vtIndex[i] = std::stoi(vert.substr(firstSlash + 1));
                    hasUV = true;
                }
                else if (secondSlash == firstSlash + 1) {
                    // Format: v//vn
                    vIndex[i] = std::stoi(vert.substr(0, firstSlash));
                    vnIndex[i] = std::stoi(vert.substr(secondSlash + 1));
                    hasNormal = true;
                }
                else {
                    // Format: v/vt/vn
                    vIndex[i] = std::stoi(vert.substr(0, firstSlash));
                    vtIndex[i] = std::stoi(vert.substr(firstSlash + 1, secondSlash - firstSlash - 1));
                    vnIndex[i] = std::stoi(vert.substr(secondSlash + 1));
                    hasUV = true;
                    hasNormal = true;
                }
            }

            for (int i = 0; i < 3; i++) {
                out_vertices.push_back(temp_vertices[vIndex[i] - 1]);
                if (hasUV) out_uvs.push_back(temp_uvs[vtIndex[i] - 1]);
                else       out_uvs.push_back(glm::vec2(0.0f));
                if (hasNormal) out_normals.push_back(temp_normals[vnIndex[i] - 1]);
                else           out_normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); // fallback normal
            }
        }
        else {
            // Skip unknown lines
            char skip[1000];
            fgets(skip, 1000, file);
        }
    }

    fclose(file);
    return true;
}
