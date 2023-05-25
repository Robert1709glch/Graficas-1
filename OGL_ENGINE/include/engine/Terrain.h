#pragma once
#include <SOIL/SOIL.h>
#include <engine/camera.h>
#include <engine/shader_m.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stb/stb_image.h>
#include <vector>
struct SimpleVertex
{
	glm::vec3 Position;
	glm::vec2 TexCoords;
	glm::vec3 Normal;
};

class Terrain
{
private:
	unsigned int VAO, VBO, EBO;
	unsigned int textures[4];
	int width, height;
	float **heights;
	std::vector<SimpleVertex> vertices;
	std::vector<int> indices;
	Shader *shader;	

	void createVertexData(const char *imagePath)
	{
		int nrChannels;
		unsigned char *image = stbi_load(imagePath, &width, &height, &nrChannels, 0);
		std::cout << "Width: " << width << " Height: " << height << std::endl;

		heights = new float *[height];
		for (int i = 0; i < height; i++)
		{
			heights[i] = new float[width];
		}

		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++)
			{
				unsigned char r = image[(col + row * width) * 3 + 0];
				unsigned char g = image[(col + row * width) * 3 + 1];
				unsigned char b = image[(col + row * width) * 3 + 2];

				int gValue = rgbToGrayscale((int)r, (int)g, (int)b);

				float xpos = ((float)col / (float)(width - 1)) - 0.5f;
				float ypos = ((float)gValue / (float)255) * 0.1;
				float zpos = ((float)row / (float)(height - 1)) - 0.5f;

				*(*(heights + row) + col) = ypos;
				SimpleVertex v;
				v.Position = glm::vec3(xpos, ypos, zpos);

				//calcula los uv's
				float s = float(col) / float(width - 1);
				float t = float(row) / float(height - 1);

				v.TexCoords = glm::vec2(s, t);
				v.Normal = glm::vec3(0);
				vertices.push_back(v);
				//std::cout << Vec3f(xpos, ypos, zpos) << std::endl;
			}
		}

		delete image;
	}

	void createIndexData()
	{
		for (int row = 0; row < height - 1; row++)
		{
			for (int col = 0; col < width - 1; col++)
			{
				int topLeftIndexNum = (int)(row * width + col);
				int topRightIndexNum = (int)(row * width + col + 1);
				int bottomLeftIndexNum = (int)((row + 1) * width + col);
				int bottomRightIndexNum = (int)((row + 1) * width + col + 1);

				// write out two triangles
				indices.push_back(topLeftIndexNum);
				indices.push_back(bottomLeftIndexNum);
				indices.push_back(topRightIndexNum);

				indices.push_back(topRightIndexNum);
				indices.push_back(bottomLeftIndexNum);
				indices.push_back(bottomRightIndexNum);
			}
		}
	}

	int rgbToGrayscale(int r, int g, int b)
	{
		//averaging comp
		return (r + g + b) / 3;
	}

public:
	Terrain()
	{

	}
	Terrain(const char *imagePath, const char **texturePaths)
	{
		createVertexData(imagePath);
		createIndexData();
		shader = new Shader("shaders/terrain.vs", "shaders/terrain.fs");

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SimpleVertex), &vertices.front(), GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices.front(), GL_STATIC_DRAW);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)offsetof(SimpleVertex, SimpleVertex::TexCoords));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)offsetof(SimpleVertex, SimpleVertex::Normal));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

		for (int i = 0; i < 4; i++)
			textures[i] = loadTextures(texturePaths[i]);

		shader->use();
		shader->setInt("texture1", 0);
		shader->setInt("texture2", 1);
		shader->setInt("texture3", 2);
		shader->setInt("texture4", 3);
	}

	unsigned int loadTextures(const char *path)
	{
		int Width, Height, nrChannels;

		unsigned char *data = stbi_load(path, &Width, &Height, &nrChannels, 0);

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLenum format = 1;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		return texture;
	}
	void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
	{
		shader->use();
		shader->setMat4("model", model);
		shader->setMat4("view", view);
		shader->setMat4("projection", projection);

		glBindVertexArray(VAO);
		//for (int i = 0; i < sizeof(textures) / sizeof(unsigned int); i++)
		//{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[0]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[1]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, textures[2]);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, textures[3]);
		//}
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);		
	}


	int getHeight()
	{
		return height;
	}

	Shader* getShader()
	{
		return shader;
	}

	int getWidth()
	{
		return width;
	}
	void Release()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		for (int i = 0; i < width; i++)
		{
			delete[] heights[i];
		}
		delete[] heights;
		delete shader;
	}
};
