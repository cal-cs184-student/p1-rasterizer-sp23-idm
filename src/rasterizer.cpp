#include "rasterizer.h"

using namespace std;

namespace CGL {

	RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
		size_t width, size_t height,
		unsigned int sample_rate) {
		this->psm = psm;
		this->lsm = lsm;
		this->width = width;
		this->height = height;
		this->sample_rate = sample_rate;

		sample_buffer.resize(width * height * sample_rate, Color::White);
	}

	// Used by rasterize_point and rasterize_line
	void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
		// TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
		// NOTE: You are not required to implement proper supersampling for points and lines
		// It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)
		for (int i = 0; i < sample_rate; i++) {
			sample_buffer[sample_rate * (y * width + x) + i] = c;
		}
	}

	// Rasterize a point: simple example to help you start familiarizing
	// yourself with the starter code.
	//
	void RasterizerImp::rasterize_point(float x, float y, Color color) {
		// fill in the nearest pixel
		int sx = (int)floor(x);
		int sy = (int)floor(y);

		// check bounds
		if (sx < 0 || sx >= width) return;
		if (sy < 0 || sy >= height) return;

		fill_pixel(sx, sy, color);
		return;
	}

	// Rasterize a line.
	void RasterizerImp::rasterize_line(float x0, float y0,
		float x1, float y1,
		Color color) {
		if (x0 > x1) {
			swap(x0, x1); swap(y0, y1);
		}

		float pt[] = { x0,y0 };
		float m = (y1 - y0) / (x1 - x0);
		float dpt[] = { 1,m };
		int steep = abs(m) > 1;
		if (steep) {
			dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
			dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
		}

		while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
			rasterize_point(pt[0], pt[1], color);
			pt[0] += dpt[0]; pt[1] += dpt[1];
		}
	}

	// Rasterize a triangle.
	void RasterizerImp::rasterize_triangle(float x0, float y0,
		float x1, float y1,
		float x2, float y2,
		Color color) {
		// TODO: Task 1: Implement basic triangle rasterization here, no supersampling

		auto inside_edge = [&](float x, float y) {
			float d0 = (y - y0) * (x1 - x0) - (x - x0) * (y1 - y0);
			float d1 = (y - y1) * (x2 - x1) - (x - x1) * (y2 - y1);
			float d2 = (y - y2) * (x0 - x2) - (x - x2) * (y0 - y2);
			return (d0 >= 0 && d1 >= 0 && d2 >= 0) || (d0 <= 0 && d1 <= 0 && d2 <= 0);
		};

		int xmin = (int)floor(min({ x0, x1, x2 }));
		int ymin = (int)floor(min({ y0, y1, y2 }));
		int xmax = (int)ceil(max({ x0, x1, x2 }));
		int ymax = (int)ceil(max({ y0, y1, y2 }));


		float stride = 1. / sqrt(sample_rate);
		// TODO: Task 2: Update to implement super-sampled rasterization
		int i;
		for (int x = xmin; x < xmax; x++) {
			for (int y = ymin; y < ymax; y++) {
				i = sample_rate * (x + width * y);
				for (float sx = 0.5 * stride; sx < 1; sx += stride) {
					for (float sy = 0.5 * stride; sy < 1; sy += stride) {
						if (inside_edge(x + sx, y + sy)) {
							sample_buffer[i] = color;
						}
						i++;
					}
				}

			}
		}
	}


	void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
		float x1, float y1, Color c1,
		float x2, float y2, Color c2)
	{
		// TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
		// Hint: You can reuse code from rasterize_triangle
		auto in_side_edge = [&](float x, float y) {
			float d0 = (y - y0) * (x1 - x0) - (x - x0) * (y1 - y0);
			float d1 = (y - y1) * (x2 - x1) - (x - x1) * (y2 - y1);
			float d2 = (y - y2) * (x0 - x2) - (x - x2) * (y0 - y2);
			return (d0 >= 0 && d1 >= 0 && d2 >= 0) || (d0 <= 0 && d1 <= 0 && d2 <= 0);
		};

		int xmin = (int)floor(min({ x0, x1, x2 }));
		int ymin = (int)floor(min({ y0, y1, y2 }));
		int xmax = (int)ceil(max({ x0, x1, x2 }));
		int ymax = (int)ceil(max({ y0, y1, y2 }));

		auto get_color = [&](float x, float y) {
			float alpha;
			float beta;
			float gamma;
			alpha = (-(x - x1) * (y2 - y1) + (y - y1) * (x2 - x1)) / (-(x0 - x1) * (y2 - y1) + (y0 - y1) * (x2 - x1));
			beta = (-(x - x2) * (y0 - y2) + (y - y2) * (x0 - x2)) / (-(x1 - x2) * (y0 - y2) + (y1 - y2) * (x0 - x2));
			gamma = 1 - alpha - beta;
			return alpha * c0 + beta * c1 + gamma * c2;
		};


		float stride = 1. / sqrt(sample_rate);
		int i;
		for (int x = xmin; x < xmax; x++) {
			for (int y = ymin; y < ymax; y++) {
				i = sample_rate * (x + width * y);
				for (float sx = 0.5 * stride; sx < 1; sx += stride) {
					for (float sy = 0.5 * stride; sy < 1; sy += stride) {
						if (in_side_edge(x + sx, y + sy)) {
							sample_buffer[i] = get_color(x, y);
						}
						i++;
					}
				}

			}
		}
	}


	void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
		float x1, float y1, float u1, float v1,
		float x2, float y2, float u2, float v2,
		Texture& tex)
	{
		// TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
		// TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
		// Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle
		auto in_side_edge = [&](float x, float y) {
			float d0 = (y - y0) * (x1 - x0) - (x - x0) * (y1 - y0);
			float d1 = (y - y1) * (x2 - x1) - (x - x1) * (y2 - y1);
			float d2 = (y - y2) * (x0 - x2) - (x - x2) * (y0 - y2);
			return (d0 >= 0 && d1 >= 0 && d2 >= 0) || (d0 <= 0 && d1 <= 0 && d2 <= 0);
		};

		int xmin = (int)floor(min({ x0, x1, x2 }));
		int ymin = (int)floor(min({ y0, y1, y2 }));
		int xmax = (int)ceil(max({ x0, x1, x2 }));
		int ymax = (int)ceil(max({ y0, y1, y2 }));
		Vector2D p0(u0, v0);

		auto barycentric = [&](float x, float y) {
			float alpha = (-(x - x1) * (y2 - y1) + (y - y1) * (x2 - x1)) / (-(x0 - x1) * (y2 - y1) + (y0 - y1) * (x2 - x1));
			float beta = (-(x - x2) * (y0 - y2) + (y - y2) * (x0 - x2)) / (-(x1 - x2) * (y0 - y2) + (y1 - y2) * (x0 - x2));
			float gamma = 1 - alpha - beta;
			float u = alpha * u0 + beta * u1 + gamma * u2;
			float v = alpha * v0 + beta * v1 + gamma * v2;
			Vector2D P(u, v);
			return P;
		};

		auto get_color_improved = [&](float x, float y) {
			SampleParams sp = {barycentric(x, y), barycentric(x+1., y), barycentric(x, y+1.), psm, lsm};
			return tex.sample(sp);
		};

		float stride = 1. / sqrt(sample_rate);
		int i;
		for (int x = xmin; x < xmax; x++) {
			for (int y = ymin; y < ymax; y++) {
				i = sample_rate * (x + width * y);
				for (float sx = 0.5 * stride; sx < 1; sx += stride) {
					for (float sy = 0.5 * stride; sy < 1; sy += stride) {
						if (in_side_edge(x + sx, y + sy)) {
							sample_buffer[i] = get_color_improved(x + sx, y + sy);
						}
						i++;
					}
				}

			}
		}

	}

	void RasterizerImp::set_sample_rate(unsigned int rate) {
		// TODO: Task 2: You may want to update this function for supersampling support

		this->sample_rate = rate;
		this->sample_buffer.resize(width * height * rate, Color::White);
	}


	void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
		size_t width, size_t height)
	{
		// TODO: Task 2: You may want to update this function for supersampling support

		this->width = width;
		this->height = height;
		this->rgb_framebuffer_target = rgb_framebuffer;
		this->sample_buffer.resize(width * height, Color::White);
	}


	void RasterizerImp::clear_buffers() {
		std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
		std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
	}


	// This function is called at the end of rasterizing all elements of the
	// SVG file.  If you use a supersample buffer to rasterize SVG elements
	// for antialising, you could use this call to fill the target framebuffer
	// pixels from the supersample buffer data.
	//
	void RasterizerImp::resolve_to_framebuffer() {
		// TODO: Task 2: You will likely want to update this function for supersampling support
		Color col = Color(0, 0, 0);
		for (int x = 0; x < width; ++x) {
			for (int y = 0; y < height; ++y) {
				col = Color(0, 0, 0);
				for (int s = 0; s < sample_rate; s++) {
					col += (1. / sample_rate) * sample_buffer[sample_rate * (y * width + x) + s];
				}
				for (int k = 0; k < 3; ++k) {
					this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255;
				}
			}
		}

	}

	Rasterizer::~Rasterizer() { }


}// CGL
