#pragma once

#include <stdint.h>
namespace Core
{
	struct point
	{
		int x, y;
		point() :x(), y() {};
		point(int& x, int& y) : x(x), y(y) {};
		point(const point& other) : x(other.x), y(other.y) {};
	};

	struct camera
	{
		point pos;
	};

	class hitbox
	{
	protected:
		point ori;
		//
		int height, len;
		uint32_t id;//bite use

	public:
		virtual void paint() = 0;
		virtual int flush() = 0;
	};

	class player :hitbox
	{
	private:
		//textureID tex;

	public:
		//void load_texture(textureID& tex) {}
		void paint() {}
	};
}