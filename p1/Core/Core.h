
namespace Core
{
	struct Point
	{
		int x, y;
		Point() :x(), y() {};
		Point(int& x, int& y) : x(x), y(y) {};
		Point(const Point& other) : x(other.x), y(other.y) {};
	};

	class 
}