
#pragma once

#include "vmath.h"
#include "gl.h"

#include <string>
#include <vector>
#include <fstream>

const u32 NUM_DATA_POINTS = 10;
const u32 NUM_PIXELS = 784;
static_assert(NUM_DATA_POINTS <= 60000, "ree");

const colorf color_table[10] = {
	color3{204, 102, 102}.to_f(),
	color3{204, 163, 102}.to_f(),
	color3{184, 204, 102}.to_f(),
	color3{122, 204, 102}.to_f(),
	color3{102, 204, 143}.to_f(),
	color3{102, 204, 204}.to_f(),
	color3{102, 143, 204}.to_f(),
	color3{122, 102, 204}.to_f(),
	color3{184, 102, 204}.to_f(),
	color3{204, 102, 163}.to_f()
};

struct dataset {
	u8  labels[NUM_DATA_POINTS] = {};
	f32 pixels[NUM_DATA_POINTS][NUM_PIXELS] = {};

	f32 distances[NUM_DATA_POINTS][NUM_DATA_POINTS] = {};

	GLuint textures[NUM_DATA_POINTS] = {};

	void load(std::string images, std::string labels);
	void transform_axis(scene& s, i32 x, i32 y, i32 z);
	void transform_opt(scene& s);
	void destroy();
};

std::vector<std::vector<f64>> run_nlopt(bool sammon, dataset *data);

void dataset::transform_axis(scene& s, i32 x, i32 y, i32 z) {

	s.clear();

	for(i32 i = 0; i < NUM_DATA_POINTS; i++) {

		s.add_data(40.0f * v3(pixels[i][x], pixels[i][y], pixels[i][z]), color_table[labels[i]], i + 1);
	}
}

void dataset::transform_opt(scene& s) {

	s.clear();

	std::vector<std::vector<f64>> result = run_nlopt(true, this);

	for(i32 i = 0; i < NUM_DATA_POINTS; i++) {

		f64 x = 0, y = 0, z = 0;
		for(i32 j = 0; j < NUM_PIXELS; j++) {
			x += result[0][j] * pixels[i][j];
			y += result[1][j] * pixels[i][j];
			z += result[2][j] * pixels[i][j];
		}

		s.add_data(v3((f32)x, (f32)y, (f32)z), color_table[labels[i]], i + 1);
	}
}

void dataset::destroy() {

	glDeleteTextures(NUM_DATA_POINTS, textures);
}

void dataset::load(std::string ifile, std::string lfile) {
	std::ifstream iin(ifile, std::ios::binary);
	std::ifstream lin(lfile, std::ios::binary);

	lin.seekg(8, std::ios_base::beg);
	lin.read((char*)labels, NUM_DATA_POINTS);

	u8* pix = new u8[NUM_DATA_POINTS * NUM_PIXELS];
	iin.seekg(16, std::ios_base::beg);
	iin.read((char*)pix, NUM_DATA_POINTS * 28 * 28);

	for(int i = 0; i < NUM_DATA_POINTS; i++) {
		for(int j = 0; j < NUM_PIXELS; j++) {
			pixels[i][j] = pix[i * NUM_PIXELS + j] / 255.0f;
		}
	}

	glGenTextures(NUM_DATA_POINTS, textures);
	for(int i = 0; i < NUM_DATA_POINTS * NUM_PIXELS; i += NUM_PIXELS) {
		
		int t = i / NUM_PIXELS;
		glBindTexture(GL_TEXTURE_2D, textures[t]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 28, 28, 0, GL_RED, GL_UNSIGNED_BYTE, &pix[i]);

		GLint swizzle[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] pix;

	for(int i = 0; i < NUM_DATA_POINTS; i++) {
		for(int j = 0; j < NUM_DATA_POINTS; j++) {
			
			f32 accum = 0.0f;
			for(int k = 0; k < NUM_PIXELS; k++) {
				accum += pixels[i][k] * pixels[j][k];
			}
			distances[i][j] = sqrtf(accum);
		}
	}
}
