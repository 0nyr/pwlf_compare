#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <math.h>
#include <sstream>
#include <string>
#include <bitset>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>   // for std::size_t

#include "json.hpp"

// This is a macro that converts any object into a string by using its operator<<(ostream&).
// Example: STR(15) == "15".
// Example: STR(vector<int>({3, 5, 7})) == "[3, 5, 7]".
#define STR(x) ([&]() { std::ostringstream qwert; qwert << (x); return qwert.str();})()


// #################### NAMESPACE START ####################
namespace goc
{

// A call to this function will throw an exception with the given 'message'.
// The purpose of this function is to standardize the way we fail in the program.
inline void fail(const std::string& message, int exit_code = 1)
{
	std::cerr << "ERROR: " << message << std::endl;
	throw std::runtime_error(message);
	exit(exit_code);
}

// ########## SEPARATOR ##########

// Prints a sequence of elements in the ostream os.
// The output format is "prefix element1, element2, ..., elementn suffix".
// Precondition: The iterated elements must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename Collection>
std::ostream& print_iterable(std::ostream& os, const Collection& collection, char prefix='{', char suffix='}')
{
	os << prefix;
	for (auto it = collection.begin(); it != collection.end(); ++it)
	{
		if (it != collection.begin()) os << ", ";
		os << *it;
	}
	os << suffix;
	return os;
}

template<typename T>
void print_padded_vectors(
    std::ostream& os,
    const std::vector<T>& vec1,
    const std::vector<T>& vec2
) {
    static_assert(std::is_floating_point<T>::value, "print_padded_vectors requires floating point types");

    // Both vectors must have the same size.
    assert(vec1.size() == vec2.size() && "Both vectors must have the same size");

    if (vec1.empty()) {
        os << "[]" << std::endl;
        os << "[]" << std::endl;
        return;
    }
    
    // Convert each element of both vectors to its string representation.
    // std::showpoint forces a decimal point.
    std::vector<std::string> str_vec1, str_vec2;
    str_vec1.reserve(vec1.size());
    str_vec2.reserve(vec2.size());
    
    for (const auto& val : vec1) {
        std::ostringstream oss;
        oss << std::showpoint << val;
        str_vec1.push_back(oss.str());
    }
    for (const auto& val : vec2) {
        std::ostringstream oss;
        oss << std::showpoint << val;
        str_vec2.push_back(oss.str());
    }
    
    // Compute the maximum integer part length and maximum fraction length
    // across both vectors.
    size_t max_int_len = 0;
    size_t max_frac_len = 0;
    
    auto update_lengths = [&](const std::string& s) {
        size_t pos = s.find('.');
        size_t int_len = (pos == std::string::npos) ? s.size() : pos;
        size_t frac_len = (pos == std::string::npos) ? 0 : s.size() - pos - 1;
        max_int_len = std::max(max_int_len, int_len);
        max_frac_len = std::max(max_frac_len, frac_len);
    };

    for (const auto& s : str_vec1) {
        update_lengths(s);
    }
    for (const auto& s : str_vec2) {
        update_lengths(s);
    }
    
    // Helper lambda to print a vector using the computed field widths.
    auto print_vector = [&](const std::vector<std::string>& sv) {
        os << "[";
        bool first = true;
        for (const auto& s : sv) {
            if (!first) {
                os << ", ";
            } else {
                first = false;
            }
            size_t pos = s.find('.');
            size_t int_len = (pos == std::string::npos) ? s.size() : pos;
            size_t frac_len = (pos == std::string::npos) ? 0 : s.size() - pos - 1;
            size_t left_padding = (max_int_len > int_len) ? (max_int_len - int_len) : 0;
            size_t right_padding = (max_frac_len > frac_len) ? (max_frac_len - frac_len) : 0;
            os << std::string(left_padding, ' ') << s << std::string(right_padding, ' ');
        }
        os << "]";
    };

    print_vector(str_vec1);
    os << std::endl;
    print_vector(str_vec2);
    os << std::endl;
}

// Prints the pair in the ostream os.
// The output format is "(first, second)".
// Precondition: The types T1, T2 must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p)
{
	return os << "(" << p.first << ", " << p.second << ")";
}

// Prints the set in the ostream os.
// The output format is "{ element1, element2, ..., elementn }".
// Precondition: The type T must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& s)
{
	return print_iterable(os, s);
}

// Prints the set in the ostream os.
// The output format is "{ element1, element2, ..., elementn }".
// Precondition: The type T must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& s)
{
	return print_iterable(os, s);
}

// Prints the map in the ostream os.
// The output format is "{ (key1, value1), (key2, value2), ..., (keyn, valuen) }".
// Precondition: The types K, V must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& m)
{
	return print_iterable(os, m);
}

// Prints the map in the ostream os.
// The output format is "{ (key1, value1), (key2, value2), ..., (keyn, valuen) }".
// Precondition: The types K, V must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& m)
{
	return print_iterable(os, m);
}

// Prints the vector in the ostream os.
// The output format is "[ element1, element2, ..., elementn ]".
// Precondition: The type T must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
	os << '[';
	for (auto it = v.begin(); it != v.end(); ++it)
	{
		if (it != v.begin()) os << ", ";
		os << *it;
	}
	os << ']';
	return os;
}

// Prints the deque in the ostream os.
// The output format is "[ element1, element2, ..., elementn ]".
// Precondition: The type T must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::deque<T>& d)
{
	return print_iterable(os, d, '[', ']');
}

// Prints the list in the ostream os.
// The output format is "[ element1, element2, ..., elementn ]".
// Precondition: The type T must have the <<(ostream&) operator implemented.
// Returns: the modified ostream.
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::list<T>& l)
{
	return print_iterable(os, l, '[', ']');
}

// Prints the bitset in the ostream os.
// The output format is "( n1, n2, ..., nk )" where n1, ..., nk are set in the bitset.
// Returns: the modified ostream.
template<unsigned long N>
std::ostream& operator<<(std::ostream& os, const std::bitset<N>& b)
{
	os << "(";
	bool first_added = true;
	for (unsigned long i = 0; i < N; ++i)
	{
		if (b.test(i))
		{
			if (!first_added) os << ", ";
			first_added = false;
			os << i;
		}
	}
	return os << ")";
}

// ########## SEPARATOR ##########

// ########## SEPARATOR ##########

// Returns: If the string 'container' includes the string 'contained'.
bool string_contains(const std::string& container, const std::string& contained);

// Returns: string s without the appearances of c.
std::string remove(const std::string& s, char c);

// Returns: A list of strings which result from splitting the string s by the delimiter.
// Example: split("Hello how are you", ' ') == ["Hello", "how", "are", "you"].
std::vector<std::string> split(const std::string& s, char delimiter=',');

// Removes the whitespaces at the beginning and end of s.
// Example: "    hello  " == "hello".
std::string trim(const std::string& s);

// ########## SEPARATOR ##########

// Represents the interface for all objects that can be printed. It automatically implements the operator<< for objects
// that implement this interface.
class Printable
{
public:
	virtual void Print(std::ostream& os) const = 0;
};

std::ostream& operator<<(std::ostream& os, const Printable& object);

// ########## SEPARATOR ##########

// Constant that represents the minimum granularity to be taken into account to compare numbers.
constexpr double EPS = 10e-7;
constexpr double EPS_SLOPE_ZERO = EPS*100; // Must be greater that classic EPS. Used with slope tests for special case where 2 slopes are close to 0.

// Constant representing a number that is bigger than the numbers that will be used.
const double INFTY = 10e50;

// Returns: if x = y with respect to the tolerance EPS.
inline bool epsilon_equal(double x, double y)
{
	return fabs(x - y) < EPS;
};

// Returns: if x != y with respect to the tolerance EPS.
inline bool epsilon_different(double x, double y)
{
	return fabs(x - y) >= EPS;
};

// Returns: if x < y with respect to the tolerance EPS.
inline bool epsilon_smaller(double x, double y)
{
	return x + EPS < y;
}

// Returns: if x <= y with respect to the tolerance EPS.
inline bool epsilon_smaller_equal(double x, double y)
{
	return x - EPS < y;
}

// Returns: if x > y with respect to the tolerance EPS.
inline bool epsilon_bigger(double x, double y)
{
	return x - EPS > y;
}

// Returns: if x >= y with respect to the tolerance EPS.
inline bool epsilon_bigger_equal(double x, double y)
{
	return x + EPS > y;
}

// Returns: the sum of all numbers in 'numbers'.
// Observation: the sum of an empty collection is 0.
inline double sum(const std::vector<double>& numbers)
{
	double a = 0.0;
	for (double n: numbers) a += n;
	return a;
}

// Returns: the sum of the value(e) for e \in v.
// Observation: the sum of an empty collection is 0.
template<typename T>
double sum(const std::vector<T>& v, const std::function<double(const T& e)>& value)
{
	double a = 0.0;
	for (auto& e: v) a += value(e);
	return a;
}

// ########## SEPARATOR ##########

// This class represents a closed interval [left, right].
class Interval : public Printable
{
public:
	// Limits of the interval.
	double left, right;
	
	// Represents the empty interval [INFTY, -INFTY].
	Interval();
	
	// Represents the interval [left, right].
	Interval(double left, double right);
	
	// Returns: if left > right.
	bool Empty() const;
	
	// Returns: if value \in [left, right] using epsilon-comparison.
	bool Includes(double value) const;
	
	// Returns: if r=[a,b]\subseteq [left, right] using epsilon-comparison.
	bool IsIncludedIn(const Interval& r) const;
	
	// Returns: if r=[a,b] \cap [left, right] using epsilon-comparison.
	bool Intersects(const Interval& r) const;
	
	// Returns: this \cap r.
	Interval Intersection(const Interval& r) const;
	
	// Returns: if the domain is [a, a] for some a.
	bool IsPoint() const;

	// Returns: the union of the intervals.
	Interval Union(const Interval& r) const;
	
	// Prints the interval.
	// Format: [left, right].
	virtual void Print(std::ostream& os) const;
	
	// Returns: if two intervals are the same.
	// We say two intervals are the same if their limits are the same or if they are empty intervals (left > right).
	bool operator==(const Interval& i) const;
	
	// Returns: if two intervals are different.
	bool operator!=(const Interval& i) const;

	// Returns the total memory footprint of *this* object, in bytes,
	// including both the fixed‐size portion (sizeof(*this)) and any
	// heap allocations (e.g. std::vector buffers).
	std::size_t memory_footprint_bytes() const;	
};

// JSON format: [left, right].
void from_json(const nlohmann::json& j, Interval& i);

void to_json(nlohmann::json& j, const Interval& i);

// ########## SEPARATOR ##########

// This class represents a point (x,y) in a 2d cartesian plane.
class Point2D : public Printable
{
public:
	double x, y;
	
	Point2D(double x=0.0, double y=0.0);
	
	virtual void Print(std::ostream& os) const;

	// Returns the total memory footprint of *this* object, in bytes,
	// including both the fixed‐size portion (sizeof(*this)) and any
	// heap allocations (e.g. std::vector buffers).
	std::size_t memory_footprint_bytes() const;	
};

// JSON format: [x, y].
void from_json(const nlohmann::json& j, Point2D& p);

void to_json(nlohmann::json& j, const Point2D& p);

// ########## SEPARATOR ##########

// This class represents a segment or bounded linear function.
// An instance can be represented by two points (p1, p2) and the function
// connecting those points.
// Invariant: p1.x <= p2.x.
class LinearFunction : public Printable
{
public:
	Interval domain, image;
	double slope, intercept;
	
	LinearFunction() = default;
	
	// Creates a linear function from p1 to p2.
	// Precondition: p1.x <= p2.x.
	LinearFunction(const Point2D& p1, const Point2D& p2);
	
	// Returns: the value of the function f(x).
	// Precondition: x \in domain.
	double Value(double x) const;
	
	// Returns: the value of the function f(x).
	// Precondition: x \in domain.
	double operator()(double x) const;
	
	// Returns: the last x such that f(x) = y.
	// Observation: if slope == 0 the function is not inversible therefore many f(x) = y might exist.
	// Precondition: y \in image.
	double PreValue(double y) const;
	
	// Returns: if this function intersects the function f inside both functions domains.
	bool Intersects(const LinearFunction& f) const;
	
	// Returns: the coordinate x such that this(x) == f(x).
	// Precondition: Intersects(*this, f).
	double Intersection(const LinearFunction& f) const;
	
	// Returns: the inverse function of this function (f) if exists, otherwise returns g(y) = max{x : f(x)=y }.
	LinearFunction Inverse() const;
	
	// Restricts the domain.
	// Returns: the restricted function.
	LinearFunction RestrictDomain(const Interval& domain) const;
	
	// Restricts the image.
	// Returns: the restricted function.
	LinearFunction RestrictImage(const Interval& image) const;
	
	// Prints the function.
	// Format: {(x1, y1)->(x2,y2)}.
	void Print(std::ostream& os) const;
	
	// Returns: if two functions have the same p1 and p2.
	bool operator==(const LinearFunction& f) const;
	
	// Returns: if two functions have different p1 and p2.
	bool operator!=(const LinearFunction& f) const;

	// Returns the total memory footprint of *this* object, in bytes,
	// including both the fixed‐size portion (sizeof(*this)) and any
	// heap allocations (e.g. std::vector buffers).
	std::size_t memory_footprint_bytes() const;	
};

// JSON format: [p1, p2].
void from_json(const nlohmann::json& j, LinearFunction& f);

void to_json(nlohmann::json& j, const LinearFunction& f);

// Returns: f.domain
inline Interval dom(const LinearFunction& f) { return f.domain; }

// Returns: f.image
inline Interval img(const LinearFunction& f) { return f.image; }

// Returns: h(x) = f(x)+g(x).
// Obs: dom(h) = dom(f) \cap dom(g).
LinearFunction operator+(const LinearFunction& f, const LinearFunction& g);

// Returns: h(x) = f(x)*g(x).
// Obs: dom(h) = dom(f) \cap dom(g).
LinearFunction operator*(const LinearFunction& f, const LinearFunction& g);

// Returns: h(x) = f(x)+a.
LinearFunction operator+(const LinearFunction& f, double a);

// Returns: h(x) = f(x)*a.
LinearFunction operator*(const LinearFunction& f, double a);

// ###### SEPARATOR ##########

// This class represents a piecewise linear function. It has a sequence of linear functions with bounded domains.
// Invariant: the linear functions are non overlapping and are increasing in domain.
// Invariant: the function is stored normalized. A function is normalized iif no two consecutive pieces have the same
// 			  slope, intercept, and share the end and beginning of their domains.
// Example: [segment1={(1,2),(2,3)},segment2={(2,3),(3,4)}] is not normalized. [segment1={(1,2),(3,4)}] is normalized.
class PWLFunction : public Printable
{
public:
	// Returns: f(x)=a with the specific domain.
	static PWLFunction ConstantFunction(double a, Interval domain);
	
	// Returns: f(x)=x with the specific domain.
	static PWLFunction IdentityFunction(Interval domain);
	
	// Creates an empty piecewise linear function.
	PWLFunction();
	
	// Creates a piecewise linear function with the specified pieces.
	PWLFunction(const std::vector<LinearFunction>& pieces);

	// Constructor for 2D continuous piecewise linear function
    // Precondition: The list of breakpoints must be sorted and unique.
	PWLFunction(
		const std::vector<double>& breakpoints,
		const std::vector<double>& values
	);
	
	// Adds the piece at the end of the function.
	// Keeps the normalization invariant automatically.
	void AddPiece(const LinearFunction& piece);
	
	// Removes the last piece from the function.
	// Precondition: PieceCount() > 0.
	void PopPiece();
	
	// Returns: if the function has no pieces.
	bool Empty() const;
	
	// Returns: the numer of pieces of the function.
	int PieceCount() const;
	
	// Returns: a vector with the function pieces.
	const std::vector<LinearFunction>& Pieces() const;
	
	// Returns: the i-th piece of the function.
	// Precondition: i > PieceCount().
	const LinearFunction& Piece(int i) const;
	
	// Returns: the i-th piece of the function.
	// Precondition: i > PieceCount().
	const LinearFunction& operator[](int i) const;
	
	// Returns: the first piece of the function.
	// Precondition: !Empty().
	const LinearFunction& FirstPiece() const;
	
	// Returns: the last piece of the function.
	// Precondition: !Empty().
	const LinearFunction& LastPiece() const;
	
	// Returns: the last piece index that includes x in its domain.
	// Precondition: x \in dom(p) for any piece p.
	int PieceIncluding(double x) const;
	
	// Returns: the smallest interval [m, M] that includes all pieces domains.
	// Observation: if Empty() then returns [INFTY, -INFTY].
	Interval Domain() const;
	
	// Returns: the smallest interval [m, M] that includes all pieces images.
	// Observation: if Empty() then returns [INFTY, -INFTY].
	Interval Image() const;
	
	// Returns: the evaluation of the piece that includes x in its domain.
	// Exception: if no piece includes x in its domain, it throws an exception.
	double Value(double x) const;
	
	// Returns: the evaluation of the piece that includes x in its domain.
	// Exception: if no piece includes x in its domain, it throws an exception.
	double operator()(double x) const;
	
	// Returns: the last x such that f(x) = y. Notice that if the function is not bijective it may contain multiple
	// x such that f(x) = y.
	// Exception: if no f(x) = y, then it throws an exception.
	double PreValue(double y) const;
	
	// Returns: the composition of this function (f) and g, i.e. fog(x) == f(g(x)).
	// Observation: the domain of the new function are those x such that g(x) \in dom(f).
	PWLFunction Compose(const PWLFunction& g) const;
	
	// Returns: the inverse of this function (f) if is inversible, otherwise returns g(y) = max{x : f(x) = y}.
	PWLFunction Inverse() const;
	
	// Restricts the domain to only the pieces included in the parameter.
	// Returns: the restricted function.
	PWLFunction RestrictDomain(const Interval& domain) const;
	
	// Restricts the image to only the pieces included in the parameter.
	// Returns: the restricted function.
	PWLFunction RestrictImage(const Interval& image) const;

	bool check_invariant() const;
	bool check_normalization() const;

	const std::pair<std::vector<double>, std::vector<double>> copy_breakpoints_and_values() const;

	// Prints the function.
	// Format: [p1, p2, ..., pn].
	virtual void Print(std::ostream& os) const;
	
	// Returns: if all the pieces of both functions are the same.
	bool operator==(const PWLFunction& f) const;
	
	// Returns: if any piece of both functions is different.
	bool operator!=(const PWLFunction& f) const;

	// Returns the total memory footprint of *this* object, in bytes,
	// including both the fixed‐size portion (sizeof(*this)) and any
	// heap allocations (e.g. std::vector buffers).
	std::size_t memory_footprint_bytes() const;

	/**
	 * @brief Computes the area under the non-decreasing continuous piecewise linear function.
	 * * This function calculates the definite integral of the function from its
	 * minimum domain value to its maximum domain value. It achieves this by
	 * summing the areas of the trapezoids formed by each linear piece and the x-axis.
	 * * For each segment from (x_i, y_i) to (x_{i+1}, y_{i+1}), the area of the
	 * trapezoid is calculated as: (y_i + y_{i+1}) * (x_{i+1} - x_i) / 2.0.
	 * * Vertical segments (where x_{i+1} - x_i == 0) correctly contribute 0 area.
	 * * @return The total area under the function. Returns 0.0 if the function is empty.
	 */
	double compute_area() const;
private:
	// Updates the image_ attribute to keep it updated after a Pop() operation.
	void UpdateImage();
	
	std::vector<LinearFunction> pieces_;
	Interval domain_, image_;
};

// JSON format: [p1, p2, ..., pn].
void from_json(const nlohmann::json& j, PWLFunction& f);

void to_json(nlohmann::json& j, const PWLFunction& f);

std::string to_string(const PWLFunction& f);

// Returns: the function h(x) = f(x)+g(x).
// Observation: only returns h(x) for x \in dom(f) \cap dom(g).
PWLFunction operator+(const PWLFunction& f, const PWLFunction& g);

// Returns: the function h(x) = f(x)-g(x).
// Observation: only returns h(x) for x \in dom(f) \cap dom(g).
PWLFunction operator-(const PWLFunction& f, const PWLFunction& g);

// Returns: the function h(x) = f(x)*g(x).
// Observation: only returns h(x) for x \in dom(f) \cap dom(g).
PWLFunction operator*(const PWLFunction& f, const PWLFunction& g);

// Returns: the function h(x) = f(x)+a.
PWLFunction operator+(const PWLFunction& f, double a);
PWLFunction operator+(double a, const PWLFunction& f);

// Returns: the function h(x) = f(x)-a.
PWLFunction operator-(const PWLFunction& f, double a);

// Returns: the function h(x) = a-f(x).
PWLFunction operator-(double a, const PWLFunction& f);

// Returns: the function h(x) = f(x)*a.
PWLFunction operator*(const PWLFunction& f, double a);
PWLFunction operator*(double a, const PWLFunction& f);

// Returns: h(x) = max(f(x), g(x)).
// Obs: If x \in dom(f), but x \not\in dom(g), then h(x) = f(x). Analogously, for the opposite case.
goc::PWLFunction Max(const goc::PWLFunction& f, const goc::PWLFunction& g);

// Returns: h(x) = max(f(x), a).
goc::PWLFunction Max(const goc::PWLFunction& f, double a);
goc::PWLFunction Max(double a, const goc::PWLFunction& f);

// Returns: h(x) = min(f(x), g(x)).
// Obs: If x \in dom(f), but x \not\in dom(g), then h(x) = f(x). Analogously, for the opposite case.
goc::PWLFunction Min(const goc::PWLFunction& f, const goc::PWLFunction& g);

// Returns: h(x) = min(f(x), a).
goc::PWLFunction Min(const goc::PWLFunction& f, double a);
goc::PWLFunction Min(double a, const goc::PWLFunction& f);

// Returns: f.domain
inline Interval dom(const PWLFunction& f) { return f.Domain(); }

// Returns: f.image
inline Interval img(const PWLFunction& f) { return f.Image(); }

} // namespace goc
// #################### NAMESPACE END ####################



// #################### NAMESPACE START ####################
namespace std
{
// Returns: i.left
inline double min(const goc::Interval& i) { return i.left; }

// Returns: i.right
inline double max(const goc::Interval& i) { return i.right; }
} // namespace std
// #################### NAMESPACE END ####################



// #################### NAMESPACE START ####################
namespace nyr
{

/**
 * ### Continuous Piecewise Linear Function (CPWLF).
 * 
 * This class represents a continuous piecewise linear function.
 * It is defined by a set of breakpoints and corresponding values.
 * The function is continuous and linear between each pair of 
 * consecutive breakpoints, and its slope is always bounded 
 * (non-infinite).
 * 
 * Note that the function is stored normalized. A function is 
 * normalized iif no two consecutive pieces have the same
 * slope, intercept, and share the end and beginning of their domains.
 * 
 * Contains `p` pieces for `b = p + 1` breakpoints.
 */
class CPWLF : public goc::Printable {

private:
    std::vector<double> breakpoints_;  // Sorted x-values
    std::vector<double> values_;       // Corresponding y-values
    goc::Interval domain_, image_;

public:
    // Returns: f(x)=a with the specific domain.
	static CPWLF make_constant(double a, goc::Interval domain);

    // Returns: f(x)=x with the specific domain.
	static CPWLF make_identity(goc::Interval domain);

    // Constructor for 2D continuous piecewise linear function
    // Precondition: The list of breakpoints must be sorted and unique.
    CPWLF(
        const std::vector<double>& breakpoints,
        const std::vector<double>& values
    );

    // Return empty
    CPWLF() = default;

    // Returns true if the function is empty (no pieces).
    bool empty() const;

    // Returns the number of pieces in the function.
    size_t nb_pieces() const;

	// Returns: the smallest interval [m, M] that includes all pieces domains.
	// Observation: if Empty() then returns [INFTY, -INFTY].
	goc::Interval compute_domain() const;
	
	// Returns: the smallest interval [m, M] that includes all pieces images.
	// Observation: if Empty() then returns [INFTY, -INFTY].
	goc::Interval compute_image() const;

    // O(log n) evaluation using binary search and linear interpolation
    // Throws std::out_of_range if x is outside the domain.
    // Precondition: x \in dom(f)
    double evaluate(double x) const;
	
	// Returns: the evaluation of the piece that includes x in its domain.
	// Exception: if no piece includes x in its domain, it throws an exception.
	double operator()(double x) const;

    // Checks if the function is well-formed.
    bool check_invariant() const;

	// Checks if the function is normalized.
	bool check_normalization() const;

	// Returns: the composition of this function (f) and g, i.e. fog(x) == f(g(x)).
	// Observation: the domain of the new function are those x such that g(x) \in dom(f).
    CPWLF compose(const CPWLF& g) const;

    goc::PWLFunction to_goc_pwl_function() const;

    void Print(std::ostream& os) const;

    // Getters
    const goc::Interval& get_domain() const { return domain_; }
    const goc::Interval& get_image() const { return image_; }
    const std::vector<double>& get_breakpoints() const { return breakpoints_; }
    const std::vector<double>& get_values() const { return values_; }	
	const std::pair<std::vector<double>, std::vector<double>> copy_breakpoints_and_values() const {
		return {breakpoints_, values_};
	}

    // Returns: if the function equals another function.
    bool operator==(const CPWLF& other) const;

	// Returns the total memory footprint of *this* object, in bytes,
	// including both the fixed‐size portion (sizeof(*this)) and any
	// heap allocations (e.g. std::vector buffers).
	std::size_t memory_footprint_bytes() const;	
};

/**
 * ### Non-Decreasing Continuous Piecewise Linear Function (CPWLF).
 * 
 * This class represents a non-decreasing continuous piecewise linear function.
 * It is defined by a set of breakpoints and corresponding values.
 * The function is continuous and linear between each pair of 
 * consecutive breakpoints. Note that its slope is always non-negative
 * and can be infinite between two breakpoints (step function).
 * 
 * Note that the function is stored normalized. A function is 
 * normalized iif no two consecutive pieces have the same
 * slope, intercept, and share the end and beginning of their domains.
 * 
 * Contains `p` pieces for `b = p + 1` breakpoints.
 */
class NDCPWLF : public goc::Printable {
public:
	const std::vector<double> xs; // Sorted x-values
    const std::vector<double> ys; // Corresponding (sorted) y-values
    // NOTE: No need to store domain_ and image_ since they are obtained in O(1) from xs_ and ys_.

    // Returns: f(x)=x with the specific domain.
	static NDCPWLF make_identity(goc::Interval domain);

    // Constructor for 2D continuous piecewise linear function
    // Precondition: The list of breakpoints must be sorted and unique.
	// Precondition: The values must be non-decreasing, but not necessarily unique (step function case).
	NDCPWLF(
		const std::vector<double> xs,
		const std::vector<double> ys
	);

    // Return empty
    NDCPWLF() = default;

    // Returns true if the function is empty (no pieces).
    bool empty() const;

    // Returns the number of pieces in the function.
    size_t nb_pieces() const;

    // O(log n) evaluation using binary search and linear interpolation
    // Throws std::out_of_range if x is outside the domain.
    // Precondition: x \in dom(f)
	// Note that if x is the domain value of a step, it returns the smallest value of the step.
    double evaluate(double x) const;
	
	// Returns: the evaluation of the piece that includes x in its domain.
	// Exception: if no piece includes x in its domain, it throws an exception.
	// Note that if x is the domain value of a step, it returns the smallest value of the step.
	double operator()(double x) const;

    // Checks if the function is well-formed, i.e. that
	// its xs and ys are non-decreasing.
	// NOTE: There can be duplicate xs and ys.
    bool check_invariant() const;

	// Checks if the function is normalized.
	bool check_normalization() const;

	// Returns: the composition of this function (f) and g, i.e. fog(x) == f(g(x)).
	// Observation: the domain of the new function are those x such that g(x) \in dom(f).
	// Note that the composition of 2 NDCPWLFs is necessarily another NDCPWLF (theorem 3, Visser et al. 2020)
    NDCPWLF compose(const NDCPWLF& g) const;

	// Composition with Visser's method, no normalization.
	NDCPWLF compose_visser(const NDCPWLF& g) const;

	// Composition with Visser's method
	// Improved with normalization.
	NDCPWLF compose_opt(const NDCPWLF& g) const;

    goc::PWLFunction to_goc_pwl_function() const;

    void Print(std::ostream& os) const;

    // Getters
    inline double get_min_domain() const {
		return xs.empty() ? goc::INFTY : xs.front();
	}
    inline double get_max_domain() const {
		return xs.empty() ? -goc::INFTY : xs.back();
	}
	inline double get_min_image() const {
		return ys.empty() ? goc::INFTY : ys.front();
	}
	inline double get_max_image() const {
		return ys.empty() ? -goc::INFTY : ys.back();
	}
	inline goc::Interval get_domain() const {
		return goc::Interval(get_min_domain(), get_max_domain());
	}
	inline goc::Interval get_image() const {
		return goc::Interval(get_min_image(), get_max_image());
	}
    const std::vector<double>& get_breakpoints() const { return xs; }
    const std::vector<double>& get_values() const { return ys; }	
	const std::pair<std::vector<double>, std::vector<double>> copy_breakpoints_and_values() const {
		return {xs, ys};
	}

    // Returns: if the function equals another function.
    bool operator==(const NDCPWLF& other) const;

	// Returns the total memory footprint of *this* object, in bytes,
	// including both the fixed‐size portion (sizeof(*this)) and any
	// heap allocations (e.g. std::vector buffers).
	std::size_t memory_footprint_bytes() const;

	/**
	 * @brief Computes the area under the non-decreasing continuous piecewise linear function.
	 * * This function calculates the definite integral of the function from its
	 * minimum domain value to its maximum domain value. It achieves this by
	 * summing the areas of the trapezoids formed by each linear piece and the x-axis.
	 * * For each segment from (x_i, y_i) to (x_{i+1}, y_{i+1}), the area of the
	 * trapezoid is calculated as: (y_i + y_{i+1}) * (x_{i+1} - x_i) / 2.0.
	 * * Vertical segments (where x_{i+1} - x_i == 0) correctly contribute 0 area.
	 * * @return The total area under the function. Returns 0.0 if the function is empty.
	 */
	double compute_area() const;
};

/// “Is empty” in the same sense as Interval::Empty().
inline bool interval_vector_empty(const std::vector<double>& v) {
    // we treat empty vector or front()>back() as “empty interval”
    return v.empty() || goc::epsilon_bigger(v.front(), v.back());
}

/// Single‐point interval?
inline bool interval_vector_is_point(const std::vector<double>& v) {
    return !v.empty() && goc::epsilon_equal(v.front(), v.back());
}

/// Returns: true if values is within the domain.
/// Precondition: the vector is sorted and has at least 1 element.
inline bool interval_vector_includes(
	const std::vector<double>& v,
	const double value
) {
	return !v.empty() &&
			goc::epsilon_smaller_equal(v.front(), value) &&
		   	goc::epsilon_smaller_equal(value, v.back());
}

/// Does [v] sit inside [w]?
inline bool interval_vector_is_included_in(
    const std::vector<double>& v,
    const std::vector<double>& w
) {
    if (v.empty()) return true; // empty is subset of anything
    if (w.empty()) return false; // non-empty is not subset of empty
    return goc::epsilon_bigger_equal(v.front(), w.front())
        && goc::epsilon_smaller_equal(v.back(),  w.back());
}

/// Do [v] and [w] overlap at all?
inline bool interval_vector_intersects(
    const std::vector<double>& v,
    const std::vector<double>& w
) {
	if (v.empty() || w.empty()) return false; // empty intervals do not intersect
	return !(goc::epsilon_smaller(v.back(), w.front()) ||
			 goc::epsilon_bigger(v.front(), w.back()));
}

/// Do [v] and [w] not overlap at all?
inline bool interval_vector_disjoints(
	const std::vector<double>& v,
	const std::vector<double>& w
) {
 if (v.empty() || w.empty()) return true; // empty intervals do not intersect
 return goc::epsilon_smaller(v.back(), w.front()) ||
	goc::epsilon_bigger(v.front(), w.back());
}

// Tests for the interval_vector functions.
void test_interval_vector_intersects();
void test_interval_vector_includes();

inline double compute_raw_slope(
	const double x1, const double y1,
	const double x2, const double y2
) {
	double slope = (y2 - y1) / (x2 - x1);
	if (slope < goc::EPS_SLOPE_ZERO) {
		slope = 0.0; // Handle horizontal segments
	}
	return slope;
}
inline double compute_raw_intercept(
 const double x1, const double y1,
 const double slope
) {
	if (slope < goc::EPS_SLOPE_ZERO) {
		return y1; // Handle horizontal segments
	}
 	return y1 - slope * x1;
}

} // namespace nyr
// #################### NAMESPACE END ####################
