#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

// Placeholder — poly_var stream manipulator not implemented.
std::ios_base& (*poly_var(const std::string& name))(std::ios_base& x) {
    (void)name;
    return [](std::ios_base& ios) -> std::ios_base& { return ios; };
}

// Sparse polynomial over doubles.
//
// Internal representation:
//   coeffs[k]  — coefficient for x^k (dense, zero for absent terms)
//   terms      — exponents of non-zero coefficients, sorted in descending order
//
// This keeps iteration over non-zero terms fast while still allowing O(1)
// random access to any coefficient by exponent.
class Polynomial {
public:
    // Zero polynomial.
    Polynomial() {
        coeffs.push_back(0.0);
    }

    // Constant (scalar) polynomial p(x) = c.
    // Non-explicit to allow implicit scalar conversion, e.g. p *= 5.
    Polynomial(double c) {  // NOLINT(google-explicit-constructor)
        coeffs.push_back(c);
        terms.push_back(0);
    }

    // Polynomial multiplication.
    friend Polynomial operator*(const Polynomial& a, const Polynomial& b) {
        Polynomial result;
        if (!a || !b) {
            result.terms.clear();
            return result;
        }
        if (a.terms.empty() || b.terms.empty()) return result;

        // Pre-allocate space up to the maximum possible degree.
        auto ita = a.terms.begin();
        auto itb = b.terms.begin();
        result.coeffs.resize(static_cast<size_t>(*ita + *itb + 1), 0.0);

        for (int ea : a.terms)
            for (int eb : b.terms)
                result[static_cast<size_t>(ea + eb)] += a.coeffs[ea] * b.coeffs[eb];

        return result;
    }

    Polynomial& operator*=(const Polynomial& other) {
        Polynomial temp = *this * other;
        if (!static_cast<bool>(temp)) {
            terms.clear();
            coeffs.clear();
        } else {
            terms  = std::move(temp.terms);
            coeffs = std::move(temp.coeffs);
        }
        return *this;
    }

    friend bool operator==(const Polynomial& a, const Polynomial& b) {
        size_t ia = 0, ib = 0;
        while (ia < a.terms.size() && ib < b.terms.size()) {
            int ea = a.terms[ia], eb = b.terms[ib];
            if (ea == eb) {
                if (a.coeffs[ea] != b.coeffs[eb]) return false;
                ++ia; ++ib;
            } else if (eb < ea) {
                if (a.coeffs[ea] != 0.0) return false;
                ++ia;
            } else {
                if (b.coeffs[eb] != 0.0) return false;
                ++ib;
            }
        }
        while (ia < a.terms.size()) { if (a.coeffs[a.terms[ia++]] != 0.0) return false; }
        while (ib < b.terms.size()) { if (b.coeffs[b.terms[ib++]] != 0.0) return false; }
        return true;
    }

    friend bool operator!=(const Polynomial& a, const Polynomial& b) { return !(a == b); }

    // Write access to the coefficient of x^exp.
    // Inserts exp into terms (maintaining descending order) if not already present.
    double& operator[](std::size_t exp) {
        if (coeffs.size() <= exp)
            coeffs.resize(exp + 1, 0.0);
        auto it = std::lower_bound(terms.begin(), terms.end(), static_cast<int>(exp),
                                   [](int a, int b) { return a > b; });
        if (it == terms.end() || *it != static_cast<int>(exp))
            terms.insert(it, static_cast<int>(exp));
        return coeffs[exp];
    }

    // Read-only coefficient access; returns 0 for absent or out-of-range exponents.
    double operator[](std::size_t exp) const {
        if (exp >= coeffs.size()) return 0.0;
        auto it = std::lower_bound(terms.begin(), terms.end(), static_cast<int>(exp),
                                   [](int a, int b) { return a > b; });
        if (it == terms.end() || *it != static_cast<int>(exp)) return 0.0;
        return coeffs[exp];
    }

    // Evaluate the polynomial at x.
    double operator()(double x) const {
        double sum = 0.0;
        for (int exp : terms)
            sum += coeffs[exp] * std::pow(x, exp);
        return sum;
    }

    // Prints in standard form, highest degree first, e.g. "x^3 + 3.5*x^1 - 10".
    // Unit coefficients (±1) are suppressed for non-constant terms.
    friend std::ostream& operator<<(std::ostream& os, const Polynomial& p) {
        if (p.terms.empty()) { os << 0; return os; }
        bool first = true;
        for (auto it = p.terms.begin(); it != p.terms.end(); ++it) {
            int    exp = *it;
            double c   = p.coeffs[exp];

            if (c < 0.0)     os << "- ";
            else if (!first) os << "+ ";

            if (c != 1.0 && c != -1.0 && exp != 0) {
                if (c < 0.0) os << -c << "*";
                else         os <<  c << "*";
            } else {
                if      (c < 0.0 && c != -1.0) os << -c;
                else if (c > 0.0 && c != 1.0)  os <<  c;
            }

            if (exp != 0) os << "x^" << exp;
            if (std::next(it) != p.terms.end()) os << " ";
            first = false;
        }
        return os;
    }

    // Returns true if the polynomial has at least one non-zero coefficient.
    explicit operator bool() const {
        for (int exp : terms)
            if (coeffs[exp] != 0.0) return true;
        return false;
    }

    bool operator!() const { return !static_cast<bool>(*this); }

    // Degree of the polynomial (highest exponent with a non-zero coefficient, or 0).
    int degree() const { return terms.empty() ? 0 : terms.front(); }

    friend bool dumpMatch(const Polynomial& p, const std::vector<double>& ref) {
        return p.coeffs == ref;
    }

private:
    std::vector<double> coeffs;  // coeffs[k] is the coefficient of x^k
    std::vector<int>    terms;   // non-zero exponents in descending order
};


// --- Tests ---

static bool smallDiff(double a, double b) {
    if (a == b) return true;
    double m = std::max(std::fabs(a), std::fabs(b));
    return (m < 1e-12) ? std::fabs(a - b) < 1e-6
                       : std::fabs(a - b) / m < 1e-6;
}

int main() {
    Polynomial a, b, c;
    std::ostringstream out;

    a[0] = -10;
    a[1] = 3.5;
    a[3] = 1;
    assert(smallDiff(a(2), 5));

    out.str(""); out << a;
    assert(out.str() == "x^3 + 3.5*x^1 - 10");

    c = -2 * a;
    assert(c.degree() == 3 && dumpMatch(c, {20.0, -7.0, -0.0, -2.0}));

    out.str(""); out << c;
    assert(out.str() == "- 2*x^3 - 7*x^1 + 20");

    out.str(""); out << b;
    assert(out.str() == "0");

    b[5] = -1;
    b[2] = 3;
    out.str(""); out << b;
    assert(out.str() == "- x^5 + 3*x^2");

    c = a * b;
    assert(c.degree() == 8
        && dumpMatch(c, {-0.0, -0.0, -30.0, 10.5, -0.0, 13.0, -3.5, 0.0, -1.0}));

    out.str(""); out << c;
    assert(out.str() == "- x^8 - 3.5*x^6 + 13*x^5 + 10.5*x^3 - 30*x^2");

    a *= 5;
    assert(a.degree() == 3 && dumpMatch(a, {-50.0, 17.5, 0.0, 5.0}));

    a *= b;
    assert(a.degree() == 8
        && dumpMatch(a, {0.0, 0.0, -150.0, 52.5, -0.0, 65.0, -17.5, -0.0, -5.0}));

    assert(a != b);
    b[5] = 0;
    assert(static_cast<bool>(b));
    assert(!!b);
    b[2] = 0;
    assert(!(a == b));

    a *= 0;
    assert(a.degree() == 0);
    assert(a == b);
    assert(!static_cast<bool>(b));
    assert(!b);

    std::cout << "All tests passed." << std::endl;
    return EXIT_SUCCESS;
}

