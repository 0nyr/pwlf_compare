#include "main.h"

using namespace std;
using namespace nlohmann;

// #################### NAMESPACE START ####################
namespace goc
{

bool string_contains(const string& container, const string& contained)
{
	return container.find(contained) != string::npos;
}

string remove(const string& s, char c)
{
	string r = "";
	for (char d: s) if (d != c) r += d;
	return r;
}

vector<string> split(const string& s, char delimiter)
{
	stringstream ss(s);
	vector<string> result;
	while( ss.good() )
	{
		string substr;
		getline(ss, substr, delimiter);
		result.push_back(substr);
	}
	return result;
}

string trim(const string& s)
{
	string t;
	for (char c: s) if (c != ' ') t.append(1u, c);
	return t;
}

// ########## SEPARATOR ##########

std::ostream& operator<<(std::ostream& os, const Printable& object)
{
	object.Print(os);
	return os;
}

// ########## SEPARATOR ##########

Interval::Interval()
{
    left = INFTY;
	right = -INFTY;
}

Interval::Interval(double left, double right) : left(left), right(right)
{
}

bool Interval::Empty() const
{
	return epsilon_bigger(left, right);
}

bool Interval::Includes(double value) const
{
	return epsilon_smaller_equal(left, value) && epsilon_bigger_equal(right, value);
}

bool Interval::IsIncludedIn(const Interval& r) const
{
	return epsilon_bigger_equal(left, r.left) && epsilon_smaller_equal(right, r.right);
}

bool Interval::Intersects(const Interval& r) const
{
	return !(epsilon_smaller(r.right, left) || epsilon_bigger(r.left, right));
}

Interval Interval::Intersection(const Interval& r) const
{
	return Interval(max(left, r.left), min(right, r.right));
}

bool Interval::IsPoint() const
{
	return epsilon_equal(left, right);
}

Interval Interval::Union(const Interval& r) const
{
	return Interval(min(left, r.left), max(right, r.right));
}

void Interval::Print(std::ostream& os) const
{
	os << "[" << left << ", " << right << "]";
}

bool Interval::operator==(const Interval& i) const
{
	if (Empty()) return i.Empty();
	return epsilon_equal(left, i.left) && epsilon_equal(right, i.right);
}

bool Interval::operator!=(const Interval& i) const
{
	return !(*this == i);
}

void from_json(const json& j, Interval& i)
{
	i.left = j[0];
	i.right = j[1];
}

void to_json(json& j, const Interval& i)
{
	j = vector<json>();
	j.push_back(i.left);
	j.push_back(i.right);
}

std::size_t Interval::memory_footprint_bytes() const {
    // just two doubles
    return sizeof(*this);
}

// ###### SEPARATOR ##########

Point2D::Point2D(double x, double y) : x(x), y(y)
{

}

void Point2D::Print(ostream& os) const
{
	os << "(" << x << "," << y << ")";
}

void from_json(const json& j, Point2D& p)
{
	p.x = j[0];
	p.y = j[1];
}

void to_json(json& j, const Point2D& p)
{
	j = vector<double>({p.x, p.y});
}

std::size_t Point2D::memory_footprint_bytes() const {
    // just two doubles
    return sizeof(*this);
}

// ###### SEPARATOR ##########

LinearFunction::LinearFunction(const Point2D& p1, const Point2D& p2)
	: domain(p1.x, p2.x), image(min(p1.y, p2.y), max(p1.y, p2.y))
{
	slope = epsilon_equal(p2.x, p1.x) ? 0.0 : (p2.y - p1.y) / (p2.x - p1.x);
	if (epsilon_equal(slope, 0.0)) slope = 0.0;
	intercept = p2.y - slope * p2.x;
	// Fix for numerical errors.
	if (domain.left != INFTY && domain.left > domain.right)
		domain.left = domain.right;
}

double LinearFunction::Value(double x) const
{
	return slope * x + intercept;
}

double LinearFunction::operator()(double x) const
{
	return Value(x);
}

double LinearFunction::PreValue(double y) const
{
	if (!image.Includes(y)) fail(STR(y) + " is not in the image " + STR(image));
	if (epsilon_equal(slope, 0.0)) return domain.right;
	return (y - intercept) / slope;
}

bool LinearFunction::Intersects(const LinearFunction& f) const
{
	// If both pieces have the same slope, check if they have the same intercept.
	if (epsilon_equal(slope, f.slope)) return epsilon_equal(intercept, f.intercept);
	// Otherwise check if intersection is inside both functions domains.
	double intersection = Intersection(f);
	return domain.Includes(intersection) && f.domain.Includes(intersection);
}

double LinearFunction::Intersection(const LinearFunction& l) const
{
	return (l.intercept - intercept) / (slope - l.slope);
}

LinearFunction LinearFunction::Inverse() const
{
	return LinearFunction({min(image), PreValue(min(image))}, {max(image), PreValue(max(image))});
}

LinearFunction LinearFunction::RestrictDomain(const Interval& domain) const
{
	double left = max(this->domain.left, domain.left);
	double right = min(this->domain.right, domain.right);
	return LinearFunction({left, Value(left)}, {right, Value(right)});
}

LinearFunction LinearFunction::RestrictImage(const Interval& image) const
{
	if (epsilon_equal(slope, 0.0))
	{
		if (image.Includes(intercept)) return *this;
		else fail("Linear function is empty");
	}
	double left = max(this->image.left, image.left);
	double right = min(this->image.right, image.right);
	double pre_left = PreValue(left), pre_right = PreValue(right);
	if (pre_left > pre_right) swap(pre_left, pre_right);
	return LinearFunction({pre_left, left}, {pre_right, right});
}

void LinearFunction::Print(ostream& os) const
{
	os << "{" << Point2D(domain.left, Value(domain.left)) << "->" << Point2D(domain.right, Value(domain.right)) << "}";
}

bool LinearFunction::operator==(const LinearFunction& f) const
{
	return domain == f.domain && image == f.image && epsilon_equal(slope, f.slope) && epsilon_equal(intercept, f.intercept);
}

bool LinearFunction::operator!=(const LinearFunction& f) const
{
	return !(*this == f);
}

void from_json(const json& j, LinearFunction& f)
{
	f = LinearFunction(j[0], j[1]);
}

void to_json(json& j, const LinearFunction& f)
{
	j = vector<Point2D>();
	j.push_back(Point2D(f.domain.left, f.Value(f.domain.left)));
	j.push_back(Point2D(f.domain.right, f.Value(f.domain.right)));
}

LinearFunction operator+(const LinearFunction& f, const LinearFunction& g)
{
	if (!f.domain.Intersects(g.domain)) return LinearFunction(Point2D(0.0, 0.0), Point2D(-1.0, 0.0));
	double l = max(f.domain.left, g.domain.left);
	double r = min(f.domain.right, g.domain.right);
	return LinearFunction(Point2D(l, f.Value(l)+g.Value(l)), Point2D(r, f.Value(r)+g.Value(r)));
}

// Returns: h(x) = f(x)*g(x).
// Precondition: dom(f) == dom(g).
LinearFunction operator*(const LinearFunction& f, const LinearFunction& g)
{
	if (!f.domain.Intersects(g.domain)) return LinearFunction(Point2D(0.0, 0.0), Point2D(-1.0, 0.0));
	double l = max(f.domain.left, g.domain.left);
	double r = min(f.domain.right, g.domain.right);
	return LinearFunction(Point2D(l, f.Value(l)*g.Value(l)), Point2D(r, f.Value(r)*g.Value(r)));
}

// Returns: h(x) = f(x)+a.
LinearFunction operator+(const LinearFunction& f, double a)
{
	return LinearFunction(Point2D(f.domain.left, f.Value(f.domain.left)+a), Point2D(f.domain.right, f.Value(f.domain.right)+a));
}

// Returns: h(x) = f(x)*a.
LinearFunction operator*(const LinearFunction& f, double a)
{
	return LinearFunction(Point2D(f.domain.left, f.Value(f.domain.left)*a), Point2D(f.domain.right, f.Value(f.domain.right)*a));
}

std::size_t LinearFunction::memory_footprint_bytes() const {
    // no dynamic members
    return sizeof(*this);
}

// ###### SEPARATOR ##########

PWLFunction PWLFunction::ConstantFunction(double a, Interval domain)
{
	PWLFunction f;
	f.AddPiece(LinearFunction(Point2D(domain.left, a), Point2D(domain.right, a)));
	return f;
}

PWLFunction PWLFunction::IdentityFunction(Interval domain)
{
	PWLFunction f;
	f.AddPiece(LinearFunction(Point2D(domain.left, domain.left), Point2D(domain.right, domain.right)));
	return f;
}

PWLFunction::PWLFunction()
{
	domain_ = image_ = {INFTY, -INFTY};
}

PWLFunction::PWLFunction(const std::vector<LinearFunction>& pieces) : PWLFunction()
{
	for (auto& p: pieces) AddPiece(p);
}

PWLFunction::PWLFunction(
	const vector<double>& breakpoints,
    const vector<double>& values
) {
	if (breakpoints.empty() || values.empty() || breakpoints.size() != values.size()) {
        throw invalid_argument("Breakpoints and values must be non-empty and of the same size.");
    }
    
    #ifndef NDEBUG
    // Check if breakpoints are sorted and unique
    for (size_t i = 1; i < breakpoints.size(); ++i) {
        if (epsilon_smaller_equal(breakpoints[i], breakpoints[i-1])) {
            throw invalid_argument("Breakpoints must be sorted and unique.");
        }
    }
    #endif

	pieces_.reserve(breakpoints.size() - 1);
	for (size_t i = 0; i < breakpoints.size() - 1; ++i)
	{
		double x_left = breakpoints[i];
		double x_right = breakpoints[i + 1];
		double y_left = values[i];
		double y_right = values[i + 1];
		
		AddPiece(LinearFunction(
			Point2D(x_left, y_left),
			Point2D(x_right, y_right)
		));
	}

	#ifndef NDEBUG
	// Check if the function is normalized.
	if (!check_invariant()) {
		throw runtime_error("PWLFunction invariant violated after construction.");
	}
	#endif
}

void PWLFunction::AddPiece(const LinearFunction& piece)
{
	// If this piece is a continuation of the last piece, then we need to merge them into one piece to have the
	// function normalized.
	bool is_continuation_of_last_piece = false;
	if (!pieces_.empty())
	{
		if (epsilon_equal(pieces_.back().domain.right, piece.domain.left))
		{
			double right_val = pieces_.back().Value(max(dom(pieces_.back())));
			double left_val = piece.Value(min(dom(piece)));
			is_continuation_of_last_piece = epsilon_equal(left_val, right_val) && (pieces_.back().domain.IsPoint() || epsilon_equal(pieces_.back().slope, piece.slope));
		}
	}
	
	if (!is_continuation_of_last_piece)
	{
		// Add the new piece if it is not a continuation.
		pieces_.push_back(piece);
	}
	else
	{
		// Merge pieces to have the function normalized.
		pieces_.back().domain.right = piece.domain.right;
		pieces_.back().image.left = min(pieces_.back().image.left, piece.image.left);
		pieces_.back().image.right = max(pieces_.back().image.right, piece.image.right);
		pieces_.back().slope = piece.slope;
		pieces_.back().intercept = piece.intercept;
	}
	
	// Update domain and image.
	if (domain_.left == INFTY) domain_.left = piece.domain.left;
	domain_.right = piece.domain.right;
	image_.left = min(image_.left, piece.image.left);
	image_.right = max(image_.right, piece.image.right);
}

void PWLFunction::PopPiece()
{
	pieces_.pop_back();
	domain_ = Empty() ? Interval(INFTY, -INFTY) : Interval(domain_.left, pieces_.back().domain.right);
	UpdateImage();
}

bool PWLFunction::Empty() const
{
	return pieces_.empty();
}

int PWLFunction::PieceCount() const
{
	return pieces_.size();
}

const vector<LinearFunction>& PWLFunction::Pieces() const
{
	return pieces_;
}

const LinearFunction& PWLFunction::Piece(int i) const
{
	return pieces_[i];
}

const LinearFunction& PWLFunction::operator[](int i) const
{
	return pieces_[i];
}

const LinearFunction& PWLFunction::FirstPiece() const
{
	return pieces_.front();
}

const LinearFunction& PWLFunction::LastPiece() const
{
	return pieces_.back();
}

int PWLFunction::PieceIncluding(double x) const
{
	// if x is outside the Domain(), throw exception.
	if (epsilon_bigger(domain_.left, x) || epsilon_smaller(domain_.right, x))
	{
		fail("PWLFunction::Value(" + STR(x) +") failed, because domain is " + STR(Domain()));
		return -1;
	}
	
	// Look for a piece that include x in their domain.
	for (int i = pieces_.size()-1; i >= 0; --i)
		if (pieces_[i].domain.Includes(x))
			return i;
	
	// The function is not continuous and x is not in the domain of any piece.
	fail("PWLFunction::Value(" + STR(x) +") failed, because x is not inside the domain of its pieces.");
	return -1;
}

Interval PWLFunction::Domain() const
{
	return domain_;
}

Interval PWLFunction::Image() const
{
	return image_;
}

double PWLFunction::Value(double x) const
{
	// if x is outside the Domain(), throw exception.
	if (epsilon_bigger(domain_.left, x) || epsilon_smaller(domain_.right, x))
	{
		fail("PWLFunction::Value(" + STR(x) +") failed, because domain is " + STR(Domain()));
		return -1;
	}
	
	// Look for a piece that include x in their domain.
	for (int i = pieces_.size()-1; i >= 0; --i)
		if (pieces_[i].domain.Includes(x))
			return pieces_[i].Value(x);
	
	// The function is not continuous and x is not in the domain of any piece.
	fail("PWLFunction::Value(" + STR(x) +") failed, because x is not inside the domain of its pieces.");
	return -1;
}

double PWLFunction::operator()(double x) const
{
	return Value(x);
}

double PWLFunction::PreValue(double y) const
{
	if (epsilon_bigger(image_.left, y) || epsilon_smaller(image_.right, y))
	{
		fail("PWLFunction::PreValue(" + STR(y) +") failed, because image is " + STR(Image()));
		return -1;
	}
	
	// Look for a piece that include x in their domain.
	for (int i = (int)pieces_.size()-1; i >= 0; --i)
		if (pieces_[i].image.Includes(y))
			return pieces_[i].PreValue(y);
	
	// The function is not continuous and x is not in the domain of any piece.
	fail("PWLFunction::PreValue(" + STR(y) +") failed, because y is not inside the domain of its pieces.");
	return -1;
}

PWLFunction PWLFunction::Compose(const PWLFunction& g) const
{
	PWLFunction fog; // fog is the composition of this function (f) and g, i.e. fog(x) == f(g(x)).
	
	auto& f = *this;
	int i = 0;
	for (int j = 0; j < g.PieceCount(); ++j)
	{
		// Put i inside bounds.
		i = max(0, min(f.PieceCount()-1, i));
		
		// If g[j] is constant, image is a single y = min(img(g[j])) = max(img(g[j])).
		if (epsilon_equal(g[j].slope, 0.0))
		{
			double y = min(img(g[j]));
			
			// Find (unique) piece f[i] with img(g[j]) \subseteq dom(f[i]) if exists.
			while (i < f.PieceCount()-1 && epsilon_smaller(max(dom(f[i])), y)) ++i;
			while (i > 0 && epsilon_bigger(min(dom(f[i])), y)) --i;
			
			// If found f[i] such that dom(f[i]) \cap img(g[j]) \neq \emptyset, add the piece to fog.
			if (i >= 0 && i < f.PieceCount() && dom(f[i]).Includes(y))
				fog.AddPiece(LinearFunction({min(dom(g[j])), f[i](y)}, {max(dom(g[j])), f[i](y)}));
		}
		// If g[j] is increasing.
		else if (epsilon_bigger(g[j].slope, 0.0))
		{
			// Find first piece f[i] such that max(dom(f[i])) >= min(img(g[j])).
			while (i > 0 && epsilon_bigger_equal(max(dom(f[i-1])), min(img(g[j])))) --i;
			while (i < f.PieceCount() && epsilon_smaller(max(dom(f[i])), min(img(g[j])))) ++i;
			
			// For each piece f[i] such that dom(f[i]) \cap img(g[j]) \neq \emptyset
			for (; i < f.PieceCount() && dom(f[i]).Intersects(img(g[j])); ++i)
			{
				// Find intersection.
				Interval inter = dom(f[i]).Intersection(img(g[j]));
				double left = g[j].PreValue(inter.left), right = g[j].PreValue(inter.right);
				fog.AddPiece(LinearFunction({left, f[i](inter.left)}, {right, f[i](inter.right)}));
			}
		}
		// If g[j] is decreasing.
		else if (epsilon_smaller(g[j].slope, 0.0))
		{
			// Find last piece f[i] such that min(dom(f[i])) <= max(img(g[j])).
			while (i < f.PieceCount()-1 && epsilon_smaller_equal(min(dom(f[i+1])), max(img(g[j])))) ++i;
			while (i > 0 && epsilon_bigger(min(dom(f[i])), max(img(g[j])))) --i;
			
			// For each piece f[i] such that dom(f[i]) \cap img(g[j]) \neq \emptyset
			for (; i >= 0 && i < f.PieceCount() && dom(f[i]).Intersects(img(g[j])); --i)
			{
				// Find intersection.
				Interval inter = dom(f[i]).Intersection(img(g[j]));
				double left = g[j].PreValue(inter.right), right = g[j].PreValue(inter.left);
				fog.AddPiece(LinearFunction({left, f[i](inter.right)}, {right, f[i](inter.left)}));
			}
		}
	}
	return fog;
}

PWLFunction PWLFunction::Inverse() const
{
	PWLFunction g;
	for (auto& p: Pieces()) g = Max(g, PWLFunction({p.Inverse()}));
	return g;
}

PWLFunction PWLFunction::RestrictDomain(const Interval& domain) const
{
	PWLFunction f;
	for (auto& p: Pieces())
	{
		if (!domain.Intersects(p.domain)) continue;
		f.AddPiece(p.RestrictDomain(domain));
	}
	return f;
}

PWLFunction PWLFunction::RestrictImage(const Interval& image) const
{
	PWLFunction f;
	for (auto& p: Pieces())
	{
		if (!image.Intersects(p.image)) continue;
		f.AddPiece(p.RestrictImage(image));
	}
	return f;
}

bool PWLFunction::check_invariant() const
{
	if (pieces_.empty()) return domain_ == Interval(INFTY, -INFTY) && image_ == Interval(INFTY, -INFTY);
	
	// Check that the domain is the union of the pieces domains.
	Interval domain = pieces_.front().domain;
	for (auto& p: pieces_) domain = domain.Union(p.domain);
	if (domain != domain_) return false;
	
	// Check that the image is the union of the pieces images.
	Interval image = pieces_.front().image;
	for (auto& p: pieces_) image = image.Union(p.image);
	if (image != image_) return false;
	
	return true;
}

bool PWLFunction::check_normalization() const {
    if (pieces_.empty()) return domain_ == Interval(INFTY, -INFTY) && image_ == Interval(INFTY, -INFTY);
    
    
    for (size_t i = 1; i < pieces_.size(); ++i)
    {
        // Check that the adjacent pieces have adjacent domains.
        if (epsilon_different(pieces_[i].domain.left, pieces_[i-1].domain.right) ||
            epsilon_different(pieces_[i].Value(pieces_[i].domain.left), pieces_[i-1].Value(pieces_[i-1].domain.right)))
        {
            std::cerr << "PWLFunction::check_normalization: pieces have non-adjacent domains "
                        << pieces_[i-1].domain << " and " << pieces_[i].domain 
                        << " at indices " << i-1 << " and " << i
                        << ". Values are " << pieces_[i-1].Value(pieces_[i-1].domain.right)
                        << " and " << pieces_[i].Value(pieces_[i].domain.left)
                        << std::endl;
            return false;
        }
        // Check that adjascent domains are of increasing values
        if (epsilon_smaller_equal(pieces_[i].domain.right, pieces_[i-1].domain.left))
        {
            std::cerr << "PWLFunction::check_normalization: pieces have non-increasing domains "
                        << pieces_[i-1].domain << " and " << pieces_[i].domain 
                        << " at indices " << i-1 << " and " << i
                        << std::endl;
            return false;
        }
    }

    // Check that no piece is the continuation of the previous piece.
    for (size_t i = 1; i < pieces_.size(); ++i)
    {
        if (epsilon_equal(pieces_[i-1].domain.right, pieces_[i].domain.left) &&
            epsilon_equal(pieces_[i-1].Value(pieces_[i-1].domain.right), pieces_[i].Value(pieces_[i].domain.left)) &&
            epsilon_equal(pieces_[i-1].slope, pieces_[i].slope))
        {   
            std::cerr << "PWLFunction::check_normalization: pieces are continuations of each other "
                        << pieces_[i-1].domain << " and " << pieces_[i].domain 
                        << " at indices " << i-1 << " and " << i
                        << ". Slopes are " << pieces_[i-1].slope << " and " << pieces_[i].slope
                        << std::endl;
            return false;
        }
    }
    
    return true;
}

const std::pair<std::vector<double>, std::vector<double>> PWLFunction::copy_breakpoints_and_values() const
{
    std::vector<double> breakpoints;
    std::vector<double> values;

    // If there are no pieces, return empty vectors.
    if (pieces_.empty())
    {
        return {breakpoints, values}; // return empty vectors
    }

    for (auto& p: pieces_)
    {
        // Note that we expect the pieces to be over a continuous domain
        // so adjascent pieces share breakpoints.
        if (breakpoints.empty())
        {
            breakpoints.push_back(p.domain.left);
            breakpoints.push_back(p.domain.right);
            values.push_back(p.Value(p.domain.left));
            values.push_back(p.Value(p.domain.right));
        }
        else
        {
            // check that the last breakpoint matches the first of the current piece
            // else throw an error.
			// NOTE: We disable this check for now.
            // if (!epsilon_equal(breakpoints.back(), p.domain.left) ||
            //     !epsilon_equal(values.back(), p.Value(p.domain.left)))
            // {
            //     std::cerr << "PWLFunction::get_breakpoints_and_values: discontinuity found between pieces. "
            //                 << "Last point: (" << breakpoints.back() << ", "
            //                 << values.back() << ") and "
            //                 << "first point of current piece: (" << p.domain.left << ", "
            //                 << p.Value(p.domain.left) << ")" << std::endl;
            //     Print(std::cerr);
            //     throw std::runtime_error("PWLFunction::get_breakpoints_and_values: discontinuity found between pieces.");
            // }
            // add the right endpoint of the current piece
            breakpoints.push_back(p.domain.right);
            values.push_back(p.Value(p.domain.right));
        }
    }
    return {breakpoints, values};
}

void PWLFunction::Print(std::ostream& os) const
{
	os << "[";
	for (int i = 0; i < PieceCount(); ++i)
	{
		if (i > 0) os << ", ";
		os << Piece(i);
	}
	os << "]";
}

bool PWLFunction::operator==(const PWLFunction& f) const
{
	if (PieceCount() != f.PieceCount()) return false;
	for (int i = 0; i < PieceCount(); ++i) if (Piece(i) != f.Piece(i)) return false;
	return true;
}

bool PWLFunction::operator!=(const PWLFunction& f) const
{
	return !(*this == f);
}

void PWLFunction::UpdateImage()
{
	image_ = {INFTY, -INFTY};
	for (auto& p: pieces_)
	{
		image_.left = min(image_.left, p.image.left);
		image_.right = max(image_.right, p.image.right);
	}
}

void from_json(const json& j, PWLFunction& f)
{
	for (auto& p: j) f.AddPiece(p);
}

void to_json(json& j, const PWLFunction& f)
{
	j = vector<LinearFunction>();
	for (auto& p: f.Pieces()) j.push_back(p);
}

string to_string(const PWLFunction& f)
{
	ostringstream msg;
	msg << "PWLFunction" << endl;

	// print domain and image intervals
	msg << setw(14) << f.Domain() << setw(14) << f.Image() << endl;
	
	vector<double> xs;
	vector<double> ys;
	for (size_t i = 0; i < f.Pieces().size(); ++i)
	{
		auto&p = f.Pieces()[i];
		xs.push_back(p.domain.left);
		xs.push_back(p.domain.right);
		ys.push_back(p.image.left);
		ys.push_back(p.image.right);
		
		if (i >= f.Pieces().size() - 1) // if last piece
		{
			msg << setw(26) << setprecision(6) << fixed << p.domain.left << setw(26) << setprecision(6) << fixed << p.image.left << endl;
			msg << setw(26) << setprecision(6) << fixed << p.domain.right << setw(26) << setprecision(6) << fixed << p.image.right << endl; 
		}
		else
		{
			msg << setw(26) << setprecision(6) << fixed << p.domain.left << setw(26) << setprecision(6) << fixed << p.image.left << endl;
		}
	}
	// print vectors, using goc/print/print_utils.h '<<'
	print_padded_vectors(msg, xs, ys);
	// msg << f.DomainBreakpoints();
	// msg << f.ImageBreakpoints();
	return msg.str();
}

PWLFunction operator+(const PWLFunction& f, const PWLFunction& g)
{
	PWLFunction h;
	int i = 0, j = 0;
	while (i < f.PieceCount() && j < g.PieceCount())
	{
		auto& pf = f.Piece(i), &pg = g.Piece(j);
		if (pf.domain.Intersects(pg.domain))
		{
			double left = max(pf.domain.left, pg.domain.left), right = min(pf.domain.right, pg.domain.right);
			h.AddPiece(LinearFunction({left, pf.Value(left)+pg.Value(left)}, {right, pf.Value(right)+pg.Value(right)}));
		}
		if (epsilon_equal(pf.domain.right, pg.domain.right)) { ++i; ++j; }
		else if (epsilon_smaller(pf.domain.right, pg.domain.right)) { ++i; }
		else { ++j; }
	}
	return h;
}

PWLFunction operator-(const PWLFunction& f, const PWLFunction& g)
{
	return f + g * -1.0;
}

PWLFunction operator*(const PWLFunction& f, const PWLFunction& g)
{
	PWLFunction h;
	int i = 0, j = 0;
	while (i < f.PieceCount() && j < g.PieceCount())
	{
		auto& pf = f.Piece(i), &pg = g.Piece(j);
		if (pf.domain.Intersects(pg.domain))
		{
			double left = max(pf.domain.left, pg.domain.left), right = min(pf.domain.right, pg.domain.right);
			h.AddPiece(LinearFunction({left, pf.Value(left)*pg.Value(left)}, {right, pf.Value(right)*pg.Value(right)}));
		}
		if (epsilon_equal(pf.domain.right, pg.domain.right)) { ++i; ++j; }
		else if (epsilon_smaller(pf.domain.right, pg.domain.right)) { ++i; }
		else { ++j; }
	}
	return h;
}

PWLFunction operator+(const PWLFunction& f, double a)
{
	return f + PWLFunction::ConstantFunction(a, f.Domain());
}

PWLFunction operator+(double a, const PWLFunction& f)
{
	return f + a;
}

PWLFunction operator-(const PWLFunction& f, double a)
{
	return f + (-1 * a);
}

PWLFunction operator-(double a, const PWLFunction& f)
{
	return f * -1.0 + a;
}

PWLFunction operator*(const PWLFunction& f, double a)
{
	return f * PWLFunction::ConstantFunction(a, f.Domain());
}

PWLFunction operator*(double a, const PWLFunction& f)
{
	return f * a;
}

PWLFunction Max(const PWLFunction& f_orig, const PWLFunction& g_orig)
{
	PWLFunction f = f_orig, g = g_orig;
	PWLFunction h;
	int i = 0, j = 0;
	while (i < f.PieceCount() && j < g.PieceCount())
	{
		auto& pf = (LinearFunction&)f.Piece(i);
		auto& pg = (LinearFunction&)g.Piece(j);
		// If pf has a part before pg.
		if (epsilon_smaller(pf.domain.left, pg.domain.left))
		{
			double l = pf.domain.left, r = min(pf.domain.right, pg.domain.left);
			h.AddPiece(LinearFunction(Point2D(l, pf.Value(l)), Point2D(r, pf.Value(r))));
			pf.domain.left = r;
			pf.image.left = pf.Value(r);
			if (epsilon_equal(r, pf.domain.right)) ++i;
		}
		else if (epsilon_smaller(pg.domain.left, pf.domain.left))
		{
			double l = pg.domain.left, r = min(pg.domain.right, pf.domain.left);
			h.AddPiece(LinearFunction(Point2D(l, pg.Value(l)), Point2D(r, pg.Value(r))));
			pg.domain.left = r;
			pg.image.left = pg.Value(r);
			if (epsilon_equal(r, pg.domain.right)) ++j;
		}
		else if (epsilon_equal(pf.domain.left, pg.domain.left))
		{
			double inter = pf.Intersection(pg);
			double l = pf.domain.left;
			double r = min(pf.domain.right, pg.domain.right);
			if (epsilon_bigger(inter, l) && epsilon_smaller(inter, r)) r = inter;
			h.AddPiece(LinearFunction(Point2D(l, max(pf.Value(l), pg.Value(l))), Point2D(r, max(pf.Value(r), pg.Value(r)))));
			pf.domain.left = r;
			pg.domain.left = r;
			if (epsilon_equal(r, pf.domain.right)) ++i;
			if (epsilon_equal(r, pg.domain.right)) ++j;
		}
	}
	for (; i < f.PieceCount(); ++i) h.AddPiece(f.Piece(i));
	for (; j < g.PieceCount(); ++j) h.AddPiece(g.Piece(j));
	return h;
}

PWLFunction Max(const PWLFunction& f, double a)
{
	return Max(f, PWLFunction::ConstantFunction(a, f.Domain()));
}

PWLFunction Max(double a, const PWLFunction& f)
{
	return Max(f, a);
}

PWLFunction Min(const PWLFunction& f, const PWLFunction& g)
{
	return Max(f*-1.0, g*-1.0)*-1.0;
}

PWLFunction Min(const PWLFunction& f, double a)
{
	return Min(f, PWLFunction::ConstantFunction(a, f.Domain()));
}

PWLFunction Min(double a, const PWLFunction& f)
{
	return Min(f, a);
}

std::size_t PWLFunction::memory_footprint_bytes() const {
    // sizeof(*this) covers pieces_ header, any other members
    return sizeof(*this)
         + pieces_.capacity() * sizeof(LinearFunction);
}

} // namespace goc
// #################### NAMESPACE END ####################



// #################### NAMESPACE START ####################
namespace nyr {

// Static factory for constant function f(x) = a
CPWLF CPWLF::make_constant(double a, goc::Interval domain) {
    if (domain.Empty()) {
        return CPWLF(); // Return empty function
    }
    
    CPWLF result;
    result.breakpoints_ = {domain.left, domain.right};
    result.values_ = {a, a};
    result.domain_ = goc::Interval(domain.left, domain.right);
    result.image_ = goc::Interval(a, a);
    return result;
}

// Static factory for identity function f(x) = x
CPWLF CPWLF::make_identity(goc::Interval domain) {
    if (domain.Empty()) {
        return CPWLF(); // Return empty function
    }
    
    CPWLF result;
    result.breakpoints_ = {domain.left, domain.right};
    result.values_ = {domain.left, domain.right};
    result.domain_ = goc::Interval(domain.left, domain.right);
    result.image_ = goc::Interval(domain.left, domain.right);
    return result;
}

// Constructor for 2D continuous piecewise linear function (CPWLF)
CPWLF::CPWLF(
    const vector<double>& breakpoints,
    const vector<double>& values
) {
    if (breakpoints.empty() || values.empty() || breakpoints.size() != values.size()) {
        throw invalid_argument("Breakpoints and values must be non-empty and of the same size.");
    }
    
    #ifndef NDEBUG
    // Check if breakpoints are sorted and unique
    for (size_t i = 1; i < breakpoints.size(); ++i) {
        if (goc::epsilon_smaller_equal(breakpoints[i], breakpoints[i-1])) {
            throw invalid_argument("Breakpoints must be sorted and unique.");
        }
    }
    #endif
    
    breakpoints_ = breakpoints;
    values_ = values;
    
    // Compute domain and image
    domain_ = compute_domain();
    image_ = compute_image();
    
    #ifndef NDEBUG
    // Check invariant
    if (!check_invariant()) {
        throw runtime_error("Invariant check failed after construction.");
    }
    #endif
}

// Check if function is empty
bool CPWLF::empty() const {
    return breakpoints_.empty();
}

// Number of pieces = breakpoints - 1
size_t CPWLF::nb_pieces() const {
    return empty() ? 0 : breakpoints_.size() - 1;
}

// Domain: [first_breakpoint, last_breakpoint] since breakpoints are sorted
goc::Interval CPWLF::compute_domain() const {
    if (empty()) {
        return goc::Interval(); // Return empty interval [INFTY, -INFTY]
    }
    return goc::Interval(breakpoints_.front(), breakpoints_.back());
}

// Image: [min_value, max_value]
goc::Interval CPWLF::compute_image() const {
    if (empty()) {
        return goc::Interval(); // Return empty interval [INFTY, -INFTY]
    }
    auto minmax = std::minmax_element(values_.begin(), values_.end());
    return goc::Interval(*minmax.first, *minmax.second);
}

// O(log n) evaluation using binary search and linear interpolation
// Throws std::out_of_range if x is outside the domain.
// Precondition: x \in dom(f)
double CPWLF::evaluate(double x) const {
    if (empty()) {
        throw std::out_of_range("Cannot evaluate empty CPWLF");
    }
    
    // Check domain bounds using epsilon comparisons
    // x < breakpoints_.front() || x > breakpoints_.back()
    if (goc::epsilon_smaller(x, breakpoints_.front()) || 
        goc::epsilon_bigger(x, breakpoints_.back())
    ) {
        throw std::out_of_range("x is outside CPWLF domain");
    }
    
    // Binary search to find the right segment
    // Find first breakpoint that is epsilon_bigger than x
    // NOTE: Do not use epsilon comparison here (better optimization)
    // and perform check after finding the segment.
    auto it = std::upper_bound(breakpoints_.begin(), breakpoints_.end(), x);
    
    // Handle edge case where upper_bound returns end()
    // NOTE: critical safety check to prevent undefined behavior 
    // when dereferencing the iterator.
    if (it == breakpoints_.end()) {
        return values_.back();
    }
    
    // Handle case where x is very close to a breakpoint
    if (goc::epsilon_equal(x, *it)) {
        size_t idx = std::distance(breakpoints_.begin(), it);
        return values_[idx];
    }
    
    // Get indices for the segment [x_i, x_{i+1}]
    size_t right_idx = std::distance(breakpoints_.begin(), it);
    size_t left_idx = right_idx - 1;
    
    // Linear interpolation between (x_i, y_i) and (x_{i+1}, y_{i+1})
    double x_left = breakpoints_[left_idx];
    double x_right = breakpoints_[right_idx];
    double y_left = values_[left_idx];
    double y_right = values_[right_idx];
    
    #ifndef NDEBUG
        // Handle degenerate case using epsilon comparison
        if (goc::epsilon_equal(x_right, x_left)) {
            return y_left;
        }
    #endif
    
    // Linear interpolation: y = y_left + (y_right - y_left) * (x - x_left) / (x_right - x_left)
    double t = (x - x_left) / (x_right - x_left);
    return y_left + t * (y_right - y_left);
}

// Evaluate the function at a given x, using the domain and image
double CPWLF::operator()(double x) const {
    return evaluate(x);
}

bool CPWLF::check_invariant() const {
    if (breakpoints_.size() != values_.size()) return false;
    if (breakpoints_.size() < 2 && !empty()) return false;

    // Check no duplicate breakpoints
    for (size_t i = 1; i < breakpoints_.size(); ++i) {
        if (goc::epsilon_equal(breakpoints_[i], breakpoints_[i-1])) {
            return false; // Duplicate breakpoints found
        }
    }
    
    // Check if breakpoints are sorted
    for (size_t i = 1; i < breakpoints_.size(); ++i) {
        if (goc::epsilon_smaller_equal(breakpoints_[i], breakpoints_[i-1])) {
            return false;
        }
    }

    // Check if stored domain is consistent with breakpoints
    if (empty()) {
        if (!domain_.Empty()) {
            return false; // Empty function should have empty domain
        }
    } else {
        if (empty() && !domain_.Empty()) {
             return false; // Non-empty domain for empty function
        } 
    }

    // Check if stored image is consistent with values
    if (empty()) {
        if (!image_.Empty()) {
            return false; // Empty function should have empty image
        }
    } else {
        auto minmax = std::minmax_element(values_.begin(), values_.end());
        if (image_.left != *minmax.first || image_.right != *minmax.second) {
            return false; // Image does not match values
        }
    }

    return true;
}

bool CPWLF::check_normalization() const {
    if (breakpoints_.empty()) return domain_ == goc::Interval() && image_ == goc::Interval();
    
    // Check that the pieces are ordered by their domain.
    for (size_t i = 1; i < breakpoints_.size(); ++i) {
        if (!goc::epsilon_bigger(breakpoints_[i], breakpoints_[i-1])) {
            std::cerr << "Breakpoints are not sorted: " 
                      << breakpoints_[i-1] << " >= " << breakpoints_[i] 
                      << " at index " << i
                      << std::endl;
            return false;
        }
    }

    // Check that no piece is the continuation of the previous piece.
    for (size_t i = 1; i < breakpoints_.size() - 1; ++i) {
        double x_prev = breakpoints_[i-1];
        double y_prev = values_[i-1];
        double x_curr = breakpoints_[i];
        double y_curr = values_[i];
        double x_next = breakpoints_[i+1];
        double y_next = values_[i+1];

        // Denominators cannot be zero because of the unique-sort step.
        double slope1 = (y_curr - y_prev) / (x_curr - x_prev);
        double slope2 = (y_next - y_curr) / (x_next - x_curr);

        // If the slopes are not different, the current breakpoint is not necessary.
        if (goc::epsilon_equal(slope1, slope2)) {
            std::cerr << "Breakpoints are not normalized: "
                      << "slope1 = " << slope1 << ", slope2 = " << slope2
                      << " at index " << i
                      << std::endl;
            return false; 
        }
    }

    return true;
}

CPWLF CPWLF::compose(const CPWLF& g) const {
    const auto& f = *this;

    // 1. Pre-check
    // If either function is empty, or if the image of g does not intersect the domain of f,
    // the composition is an empty function.
    if (f.empty() || g.empty() || !f.domain_.Intersects(g.image_)) {
        return CPWLF();
    }

    std::vector<double> potential_bkpts;

    // 2. Collect Breakpoints
    // Source A: Add all breakpoints from g.
    for (double bkpt : g.breakpoints_) {
        potential_bkpts.push_back(bkpt);
    }
    
    // Source B: Find where g(x) hits a breakpoint of f.
    for (double f_y_bkpt : f.breakpoints_) {
        // We need to solve g(x) = f_y_bkpt for each piece of g.
        for (size_t i = 0; i < g.nb_pieces(); ++i) {
            double g_x1 = g.breakpoints_[i];
            double g_x2 = g.breakpoints_[i+1];
            double g_y1 = g.values_[i];
            double g_y2 = g.values_[i+1];

            // Check if the target y-value is within the vertical range of this piece of g, using epsilon comparisons.
            double min_gy = std::min(g_y1, g_y2);
            double max_gy = std::max(g_y1, g_y2);
            if (goc::epsilon_bigger_equal(f_y_bkpt, min_gy) && goc::epsilon_smaller_equal(f_y_bkpt, max_gy)) {
                // If the piece is horizontal
                if (goc::epsilon_equal(g_y1, g_y2)) {
                    // If g(x) is constant and this constant is a breakpoint of f,
                    // the endpoints of this segment are already included from g.breakpoints_.
                    continue;
                }

                // Solve for x using inverse linear interpolation.
                // f_y_bkpt = g_y1 + (g_y2 - g_y1) * (x - g_x1) / (g_x2 - g_x1)
                double x_sol = g_x1 + (f_y_bkpt - g_y1) * (g_x2 - g_x1) / (g_y2 - g_y1);
                
                // Add the solution if it's within the piece's domain interval [g_x1, g_x2], using epsilon comparisons.
                if (goc::epsilon_bigger_equal(x_sol, g_x1) && goc::epsilon_smaller_equal(x_sol, g_x2)) {
                    potential_bkpts.push_back(x_sol);
                }
            }
        }
    }
    
    // 3. Sort and Unify
    std::sort(potential_bkpts.begin(), potential_bkpts.end());
    potential_bkpts.erase(
        std::unique(potential_bkpts.begin(), potential_bkpts.end(), 
                    [](double a, double b) { return goc::epsilon_equal(a, b); }),
        potential_bkpts.end()
    );

    // 4. Filter, Evaluate, and Build Raw Function
    CPWLF raw_fog;
    for (double x_bkpt : potential_bkpts) {
        // A breakpoint is valid only if g(x) is in the domain of f.
        // g.evaluate() and f.domain_.Includes() already handle boundary cases correctly.
        double g_val = g.evaluate(x_bkpt);
        if (f.domain_.Includes(g_val)) {
            raw_fog.breakpoints_.push_back(x_bkpt);
            raw_fog.values_.push_back(f.evaluate(g_val));
        }
    }

    if (raw_fog.empty() || raw_fog.nb_pieces() == 0) {
        return CPWLF();
    }

    // 5. Normalize
    // Remove intermediate breakpoints if the slope doesn't change.
    CPWLF fog;
    if (raw_fog.nb_pieces() < 2) {
        fog = raw_fog;
    } else {
        fog.breakpoints_.push_back(raw_fog.breakpoints_.front());
        fog.values_.push_back(raw_fog.values_.front());

        for (size_t i = 1; i < raw_fog.nb_pieces(); ++i) {
            double x_prev = raw_fog.breakpoints_[i-1];
            double y_prev = raw_fog.values_[i-1];
            double x_curr = raw_fog.breakpoints_[i];
            double y_curr = raw_fog.values_[i];
            double x_next = raw_fog.breakpoints_[i+1];
            double y_next = raw_fog.values_[i+1];

            // Denominators cannot be zero because of the unique-sort step.
            double slope1 = (y_curr - y_prev) / (x_curr - x_prev);
            double slope2 = (y_next - y_curr) / (x_next - x_curr);

            // If the slopes are different, the current breakpoint is necessary.
            if (!goc::epsilon_equal(slope1, slope2)) {
                fog.breakpoints_.push_back(x_curr);
                fog.values_.push_back(y_curr);
            }
        }
        
        fog.breakpoints_.push_back(raw_fog.breakpoints_.back());
        fog.values_.push_back(raw_fog.values_.back());
    }

    // 6. Construct: Update cached domain and image
    if (!fog.empty()) {
        fog.domain_ = fog.compute_domain();
        fog.image_ = fog.compute_image();
    }

    return fog;
}

void CPWLF::Print(std::ostream& os) const {
    clog << "CPWLF Image: " << image_ << ", Domain: " << domain_ << "]" << endl;
    goc::print_padded_vectors(clog, breakpoints_, values_);
}

bool CPWLF::operator==(const CPWLF& other) const {
    // Check if breakpoints and values are the same
    return (
        (breakpoints_ == other.breakpoints_) && 
        (values_ == other.values_)
    );
}

goc::PWLFunction CPWLF::to_goc_pwl_function() const {
    goc::PWLFunction pwlf;
    if (empty()) {
        return pwlf; // Return empty PWLFunction
    }
    
    // Convert breakpoints and values to goc::PWLFunction
    for (size_t i = 0; i < breakpoints_.size() - 1; ++i) {
        pwlf.AddPiece(goc::LinearFunction(
            goc::Point2D(breakpoints_[i], values_[i]),
            goc::Point2D(breakpoints_[i + 1], values_[i + 1])
        ));
    }
    
    return pwlf;
}

std::size_t nyr::CPWLF::memory_footprint_bytes() const {
    // sizeof(*this) covers: 
    //   - domain_, image_, the two vector headers (3 pointers each on most platforms), etc.
    // the following adds the heap‐allocated storage for both vectors:
    return sizeof(*this)
         + breakpoints_.capacity() * sizeof(double)
         + values_.capacity()      * sizeof(double);
}

// ########## SEPARATOR ##########

// Check if function is empty
bool nyr::NDCPWLF::empty() const {
    return xs_.empty();
}

nyr::NDCPWLF nyr::NDCPWLF::compose(const nyr::NDCPWLF& g) const {
	const auto& f = *this;

	// 1. Pre-check
	// If either function is empty, or if the image of g does not intersect the domain
	// of f, the composition is an empty function.
	if (f.empty() || g.empty() ||
		// check if the image of g intersects the domain of f
		// Not ( g.max_img] [f.min_dom or f.max_dom] [g.min_img )
		!(goc::epsilon_smaller(g.ys_.back(), f.xs_.front()) || goc::epsilon_bigger(g.ys_.front(), f.xs_.back()))
	) {
		return nyr::NDCPWLF(); // Return empty function
	}

	std::vector<double> fog_xs;
	std::vector<double> fog_ys;
	// By theorem, fog has at most f.xs_.size() * g.ys_.size() points.
	// So reserve space for the worst case, and shrink later.
	fog_xs.reserve(f.xs_.size() * g.ys_.size());
	fog_ys.reserve(f.xs_.size() * g.ys_.size());
	unsigned int nb_points = 0;

	// Steps 1 and 2: Find all points P'(x', y') in fog.
	// NOTE: Since fog is a NDCPWLF, we can add x' and y' in any order
	// then sort fog_xs and fog_ys at the end.

	// 2. Iterate over g points (forward: x' given, need to compute y')
	// Each point P(x, y) in g such that y \in f.domain adds (at least) a point to fog
	// Edge case 1: If f(y) corresponds to a step (vertical segment) on f, add 2 points to fog corresponding to the segment endpoints.
	for (size_t i = 0; i < g.ys_.size(); ++i)
	{
		double y = g.ys_[i];
		if (!interval_vector_includes(f.xs_, y)) continue; // If y is not in f's domain, skip

		// Find the piece of f that contains y
		// Here, it consists in finding the first index j such that f.xs_[j] <= y <= f.xs_[j+1


		// If the piece is horizontal, add both endpoints

		// Otherwise, compute the x value corresponding to y
		
	}

	// 3. Iterate over f points: (backward: y' given, need to compute x')
	// Each point P(x, y) in f such that x \in g.image adds (a maximum of) a point to fog
	// Edge case 1: If x corresponds to a step (vertical segment) on g, then this point has already been added in the previous step (continue)
	// Edge case 2: If x corresponds precisely to a point on g (i.e. x is in g.ys_), then this point has already been added in the previous step (continue)

	// 4. Sort fog_xs and fog_ys

	return nyr::NDCPWLF(fog_xs, fog_ys);
}



// #################### NAMESPACE END ####################

} // namespace nyr



