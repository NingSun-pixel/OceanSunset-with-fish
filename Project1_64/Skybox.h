#ifndef SKYBOX_H
#define SKYBOX_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <stb_image.h>


GLuint loadCubemap(std::vector<std::string> faces);
void renderSkybox(GLuint skyboxShader, GLuint skyboxVAO, GLuint cubemapTexture, glm::mat4 view, glm::mat4 projection);

#endif // SKYBOX_H
