#pragma once

// Include most of the C standard library for convenience
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>

////////////////////
// Handy macros
////////////////////

/** Concatenates two literals or two macros
Example:
	#define COUNT 42
	CONCAT(myVariable, COUNT)
expands to
	myVariable42
*/
#define CONCAT_LITERAL(x, y) x ## y
#define CONCAT(x, y) CONCAT_LITERAL(x, y)

/** Surrounds raw text with quotes
Example:
	#define NAME "world"
	printf("Hello " TOSTRING(NAME))
expands to
	printf("Hello " "world")
and of course the C++ lexer/parser then concatenates the string literals.
*/
#define TOSTRING_LITERAL(x) #x
#define TOSTRING(x) TOSTRING_LITERAL(x)

/** Produces the length of a static array in number of elements */
#define LENGTHOF(arr) (sizeof(arr) / sizeof((arr)[0]))

/** Reserve space for `count` enums starting with `name`.
Example:
	enum Foo {
		ENUMS(BAR, 14),
		BAZ
	};

	BAR + 0 to BAR + 13 is reserved. BAZ has a value of 14.
*/
#define ENUMS(name, count) name, name ## _LAST = name + (count) - 1

/** Deprecation notice for GCC */
#ifdef WIN32
#define DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED __attribute__ ((deprecated))
#endif // 




/** References binary files compiled into the program.
For example, to include a file "Test.dat" directly into your program binary, add
	BINARIES += Test.dat
to your Makefile and declare
	BINARY(Test_dat);
at the root of a .c or .cpp source file. Note that special characters are replaced with "_". Then use
	BINARY_START(Test_dat)
	BINARY_END(Test_dat)
to reference the data beginning and end as a void* array, and
	BINARY_SIZE(Test_dat)
to get its size in bytes.
*/
#ifdef ARCH_MAC
	// Use output from `xxd -i`
	#define BINARY(sym) extern unsigned char sym[]; extern unsigned int sym##_len
	#define BINARY_START(sym) ((const void*) sym)
	#define BINARY_END(sym) ((const void*) sym + sym##_len)
	#define BINARY_SIZE(sym) (sym##_len)
#else
	#define BINARY(sym) extern char _binary_##sym##_start, _binary_##sym##_end, _binary_##sym##_size
	#define BINARY_START(sym) ((const void*) &_binary_##sym##_start)
	#define BINARY_END(sym) ((const void*) &_binary_##sym##_end)
	// The symbol "_binary_##sym##_size" doesn't seem to be valid after a plugin is dynamically loaded, so simply take the difference between the two addresses.
	#define BINARY_SIZE(sym) ((size_t) (&_binary_##sym##_end - &_binary_##sym##_start))
#endif




namespace rack {

    ////////////////////
    // basic integer functions
    ////////////////////

    /** Returns true if x is odd */
    inline bool isOdd(int x) {
        return x % 2 != 0;
    }

    /** Returns true if x is odd */
    inline bool isEven(int x) {
        return x % 2 == 0;
    }

    /** Returns the minimum of `a` and `b` */
    inline int min_rack(int a, int b) {
        return (a < b) ? a : b;
    }

    /** Returns the maximum of `a` and `b` */
    inline int max_rack(int a, int b) {
        return (a > b) ? a : b;
    }

    /** Limits `x` between `a` and `b`
    Assumes a <= b
    */
    inline int clamp(int x, int a, int b) {
        return min_rack(max_rack(x, a), b);
    }

    /** Limits `x` between `a` and `b`
    If a > b, switches the two values
    */
    inline int clamp2(int x, int a, int b) {
        return clamp(x, min_rack(a, b), max_rack(a, b));
    }

    /** Euclidean modulus, always returns 0 <= mod < base for positive base.
    */
    inline int eucmod(int a, int base) {
        int mod = a % base;
        return (mod >= 0) ? mod : mod + base;
    }

    /** Returns floor(log_2(n)), or 0 if n == 1.
    */
    inline int log2(int n) {
        int i = 0;
        while (n >>= 1) {
            i++;
        }
        return i;
    }

    inline bool ispow2(int n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    ////////////////////
    // basic float functions
    ////////////////////

    /** Returns the min_rackimum of `a` and `b` */
    inline float min_rack(float a, float b) {
        return (a < b) ? a : b;
    }

    /** Returns the max_rackimum of `a` and `b` */
    inline float max_rack(float a, float b) {
        return (a > b) ? a : b;
    }

    /** Limits `x` between `a` and `b`
    Assumes a <= b
    */
    inline float clamp(float x, float a, float b) {
        return min_rack(max_rack(x, a), b);
    }

    /** Limits `x` between `a` and `b`
    If a > b, switches the two values
    */
    inline float clamp2(float x, float a, float b) {
        return clamp(x, min_rack(a, b), max_rack(a, b));
    }

    /** Returns 1.f for positive numbers and -1.f for negative numbers (including positive/negative zero) */
    inline float sgn(float x) {
        return copysignf(1.0f, x);
    }

    inline float eucmod(float a, float base) {
        float mod = fmodf(a, base);
        return (mod >= 0.0f) ? mod : mod + base;
    }

    inline bool isNear(float a, float b, float epsilon = 1.0e-6f) {
        return fabsf(a - b) <= epsilon;
    }

    /** If the magnitude of x if less than eps, return 0 */
    inline float chop(float x, float eps) {
        return (-eps < x && x < eps) ? 0.0f : x;
    }

    inline float rescale(float x, float a, float b, float yMin, float yMax) {
        return yMin + (x - a) / (b - a) * (yMax - yMin);
    }

    inline float crossfade(float a, float b, float frac) {
        return a + frac * (b - a);
    }

    /** Linearly interpolate an array `p` with index `x`
    Assumes that the array at `p` is of length at least floor(x)+1.
    */
    inline float interpolateLinear(const float *p, float x) {
        int xi = x;
        float xf = x - xi;
        return crossfade(p[xi], p[xi + 1], xf);
    }

    /** Complex multiply c = a * b
    Arguments may be the same pointers
    i.e. cmultf(&ar, &ai, ar, ai, br, bi)
    */
    inline void cmult(float *cr, float *ci, float ar, float ai, float br, float bi) {
        *cr = ar * br - ai * bi;
        *ci = ar * bi + ai * br;
    }

    ////////////////////
    // 2D vector and rectangle
    ////////////////////

    struct Rect;

    struct Vec {
        float x = 0.f;
        float y = 0.f;

        Vec() {}
        Vec(float x, float y) : x(x), y(y) {}

        Vec neg() {
            return Vec(-x, -y);
        }
        Vec plus(Vec b) {
            return Vec(x + b.x, y + b.y);
        }
        Vec minus(Vec b) {
            return Vec(x - b.x, y - b.y);
        }
        Vec mult(float s) {
            return Vec(x * s, y * s);
        }
        Vec mult(Vec b) {
            return Vec(x * b.x, y * b.y);
        }
        Vec div(float s) {
            return Vec(x / s, y / s);
        }
        Vec div(Vec b) {
            return Vec(x / b.x, y / b.y);
        }
        float dot(Vec b) {
            return x * b.x + y * b.y;
        }
        float norm() {
            return hypotf(x, y);
        }
        Vec flip() {
            return Vec(y, x);
        }
        Vec min_rack(Vec b) {
            return Vec(rack::min_rack(x, b.x), rack::min_rack(y, b.y));
        }
        Vec max_rack(Vec b) {
            return Vec(rack::max_rack(x, b.x), rack::max_rack(y, b.y));
        }
        Vec round() {
            return Vec(roundf(x), roundf(y));
        }
        Vec floor() {
            return Vec(floorf(x), floorf(y));
        }
        Vec ceil() {
            return Vec(ceilf(x), ceilf(y));
        }
        bool isEqual(Vec b) {
            return x == b.x && y == b.y;
        }
        bool isZero() {
            return x == 0.0f && y == 0.0f;
        }
        bool isFinite() {
            return isfinite(x) && isfinite(y);
        }
        Vec clamp(Rect bound);
        Vec clamp2(Rect bound);
    };


    struct Rect {
        Vec pos;
        Vec size;

        Rect() {}
        Rect(Vec pos, Vec size) : pos(pos), size(size) {}
        /** Constructs a Rect from the upper-left position `a` and lower-right pos `b` */
        static Rect fromMinMax(Vec a, Vec b) {
            return Rect(a, b.minus(a));
        }

        /** Returns whether this Rect contains an entire point, inclusive on the top/left, non-inclusive on the bottom/right */
        bool contains(Vec v) {
            return pos.x <= v.x && v.x < pos.x + size.x
                && pos.y <= v.y && v.y < pos.y + size.y;
        }
        /** Returns whether this Rect contains an entire Rect */
        bool contains(Rect r) {
            return pos.x <= r.pos.x && r.pos.x + r.size.x <= pos.x + size.x
                && pos.y <= r.pos.y && r.pos.y + r.size.y <= pos.y + size.y;
        }
        /** Returns whether this Rect overlaps with another Rect */
        bool intersects(Rect r) {
            return (pos.x + size.x > r.pos.x && r.pos.x + r.size.x > pos.x)
                && (pos.y + size.y > r.pos.y && r.pos.y + r.size.y > pos.y);
        }
        bool isEqual(Rect r) {
            return pos.isEqual(r.pos) && size.isEqual(r.size);
        }
        Vec getCenter() {
            return pos.plus(size.mult(0.5f));
        }
        Vec getTopRight() {
            return pos.plus(Vec(size.x, 0.0f));
        }
        Vec getBottomLeft() {
            return pos.plus(Vec(0.0f, size.y));
        }
        Vec getBottomRight() {
            return pos.plus(size);
        }
        /** Clamps the edges of the rectangle to fit within a bound */
        Rect clamp(Rect bound) {
            Rect r;
            r.pos.x = clamp2(pos.x, bound.pos.x, bound.pos.x + bound.size.x);
            r.pos.y = clamp2(pos.y, bound.pos.y, bound.pos.y + bound.size.y);
            r.size.x = rack::clamp(pos.x + size.x, bound.pos.x, bound.pos.x + bound.size.x) - r.pos.x;
            r.size.y = rack::clamp(pos.y + size.y, bound.pos.y, bound.pos.y + bound.size.y) - r.pos.y;
            return r;
        }
        /** Nudges the position to fix inside a bounding box */
        Rect nudge(Rect bound) {
            Rect r;
            r.size = size;
            r.pos.x = clamp2(pos.x, bound.pos.x, bound.pos.x + bound.size.x - size.x);
            r.pos.y = clamp2(pos.y, bound.pos.y, bound.pos.y + bound.size.y - size.y);
            return r;
        }
        /** Expands this Rect to contain `other` */
        Rect expand(Rect other) {
            Rect r;
            r.pos.x = min_rack(pos.x, other.pos.x);
            r.pos.y = min_rack(pos.y, other.pos.y);
            r.size.x = max_rack(pos.x + size.x, other.pos.x + other.size.x) - r.pos.x;
            r.size.y = max_rack(pos.y + size.y, other.pos.y + other.size.y) - r.pos.y;
            return r;
        }
        /** Returns a Rect with its position set to zero */
        Rect zeroPos() {
            Rect r;
            r.size = size;
            return r;
        }
        Rect grow(Vec delta) {
            Rect r;
            r.pos = pos.minus(delta);
            r.size = size.plus(delta.mult(2.f));
            return r;
        }
        Rect shrink(Vec delta) {
            Rect r;
            r.pos = pos.plus(delta);
            r.size = size.minus(delta.mult(2.f));
            return r;
        }
    };


    inline Vec Vec::clamp(Rect bound) {
        return Vec(
            rack::clamp(x, bound.pos.x, bound.pos.x + bound.size.x),
            rack::clamp(y, bound.pos.y, bound.pos.y + bound.size.y));
    }

    inline Vec Vec::clamp2(Rect bound) {
        return Vec(
            rack::clamp2(x, bound.pos.x, bound.pos.x + bound.size.x),
            rack::clamp2(y, bound.pos.y, bound.pos.y + bound.size.y));
    }


    ////////////////////
    // Deprecated functions
    ////////////////////

    DEPRECATED inline int mini(int a, int b) { return min_rack(a, b); }
    DEPRECATED inline int maxi(int a, int b) { return max_rack(a, b); }
    DEPRECATED inline int clampi(int x, int min, int max) { return clamp(x, min, max); }
    DEPRECATED inline int absi(int a) { return abs(a); }
    DEPRECATED inline int eucmodi(int a, int base) { return eucmod(a, base); }
    DEPRECATED inline int log2i(int n) { return log2(n); }
    DEPRECATED inline bool ispow2i(int n) { return ispow2(n); }
    DEPRECATED inline float absf(float x) { return fabsf(x); }
    DEPRECATED inline float sgnf(float x) { return sgn(x); }
    DEPRECATED inline float eucmodf(float a, float base) { return eucmod(a, base); }
    DEPRECATED inline bool nearf(float a, float b, float epsilon = 1.0e-6f) { return isNear(a, b, epsilon); }
    DEPRECATED inline float clampf(float x, float min, float max) { return clamp(x, min, max); }
    DEPRECATED inline float clamp2f(float x, float min, float max) { return clamp2(x, min, max); }
    DEPRECATED inline float chopf(float x, float eps) { return chop(x, eps); }
    DEPRECATED inline float rescalef(float x, float a, float b, float yMin, float yMax) { return rescale(x, a, b, yMin, yMax); }
    DEPRECATED inline float crossf(float a, float b, float frac) { return crossfade(a, b, frac); }
    DEPRECATED inline float interpf(const float *p, float x) { return interpolateLinear(p, x); }
    DEPRECATED inline void cmultf(float *cr, float *ci, float ar, float ai, float br, float bi) { return cmult(cr, ci, ar, ai, br, bi); }


} // namespace rack


namespace rack {

////////////////////
// Template hacks
////////////////////

/** C#-style property constructor
Example:
	Foo *foo = construct<Foo>(&Foo::greeting, "Hello world");
*/
template<typename T>
T *construct() {
	return new T();
}

template<typename T, typename F, typename V, typename... Args>
T *construct(F f, V v, Args... args) {
	T *o = construct<T>(args...);
	o->*f = v;
	return o;
}

/** Defers code until the scope is destructed
From http://www.gingerbill.org/article/defer-in-cpp.html

Example:
	file = fopen(...);
	defer({
		fclose(file);
	});
*/
template<typename F>
struct DeferWrapper {
	F f;
	DeferWrapper(F f) : f(f) {}
	~DeferWrapper() { f(); }
};

template<typename F>
DeferWrapper<F> deferWrapper(F f) {
	return DeferWrapper<F>(f);
}

#define defer(code) auto CONCAT(_defer_, __COUNTER__) = deferWrapper([&]() code)

////////////////////
// Random number generator
// random.cpp
////////////////////

/** Seeds the RNG with the current time */
void randomInit();
/** Returns a uniform random uint32_t from 0 to UINT32_MAX */
uint32_t randomu32();
uint64_t randomu64();
/** Returns a uniform random float in the interval [0.0, 1.0) */
float randomUniform();
/** Returns a normal random number with mean 0 and standard deviation 1 */
float randomNormal();

DEPRECATED inline float randomf() {return randomUniform();}

////////////////////
// String utilities
// string.cpp
////////////////////

/** Converts a printf format string and optional arguments into a std::string */
std::string stringf(const char *format, ...);
std::string stringLowercase(std::string s);
std::string stringUppercase(std::string s);

/** Truncates and adds "..." to a string, not exceeding `len` characters */
std::string stringEllipsize(std::string s, size_t len);
bool stringStartsWith(std::string str, std::string prefix);
bool stringEndsWith(std::string str, std::string suffix);

/** Extracts portions of a path */
std::string stringDirectory(std::string path);
std::string stringFilename(std::string path);
std::string stringExtension(std::string path);

struct StringCaseInsensitiveCompare {
	bool operator()(const std::string &a, const std::string &b) const {
		return stringLowercase(a) < stringLowercase(b);
	}
};

////////////////////
// Operating-system specific utilities
// system.cpp
////////////////////

std::vector<std::string> systemListEntries(std::string path);
bool systemIsFile(std::string path);
bool systemIsDirectory(std::string path);
void systemCopy(std::string srcPath, std::string destPath);
void systemCreateDirectory(std::string path);

/** Opens a URL, also happens to work with PDFs and folders.
Shell injection is possible, so make sure the URL is trusted or hard coded.
May block, so open in a new thread.
*/
void systemOpenBrowser(std::string url);

////////////////////
// Debug logger
// logger.cpp
////////////////////

enum LoggerLevel {
	DEBUG_LEVEL = 0,
	INFO_LEVEL,
	WARN_LEVEL,
	FATAL_LEVEL
};

void loggerInit(bool devMode);
void loggerDestroy();
/** Do not use this function directly. Use the macros below. */
void loggerLog(LoggerLevel level, const char *file, int line, const char *format, ...);
/** Example usage:
	debug("error: %d", errno);
will print something like
	[0.123 debug myfile.cpp:45] error: 67
*/
#define debug(format, ...) loggerLog(DEBUG_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define info(format, ...) loggerLog(INFO_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define warn(format, ...) loggerLog(WARN_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define fatal(format, ...) loggerLog(FATAL_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)

////////////////////
// Thread functions
////////////////////

/** Threads which obtain a VIPLock will cause wait() to block for other less important threads.
This does not provide the VIPs with an exclusive lock. That should be left up to another mutex shared between the less important thread.
*/
struct VIPMutex {
	int count = 0;
	std::condition_variable cv;
	std::mutex countMutex;

	/** Blocks until there are no remaining VIPLocks */
	void wait() {
		std::unique_lock<std::mutex> lock(countMutex);
		while (count > 0)
			cv.wait(lock);
	}
};

struct VIPLock {
	VIPMutex &m;
	VIPLock(VIPMutex &m) : m(m) {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count++;
	}
	~VIPLock() {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count--;
		lock.unlock();
		m.cv.notify_all();
	}
};


} // namespace rack
