#pragma once

#include <string>
#include <GLAD/glad.h>
//TODO: figure out how to move this so we can include this header elsewhere if needed
#define STB_IMAGE_IMPLEMENTATION
#include <StbImg.h>
#include <iostream>

using namespace std;

class Texture {

	unsigned int m_id;

public:

	Texture(string pathToImg, GLenum wrap, GLenum filter, GLenum colorFormat) {
        glGenTextures(1, &m_id);
        Bind();
        // Wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
        // Filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        // Load
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(pathToImg.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, width, height, 0, colorFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            cout << "Failed to load texture" << endl;
        }
        stbi_image_free(data);
	}

    void Bind() {
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    unsigned int GetID() {
        return m_id;
    }
};