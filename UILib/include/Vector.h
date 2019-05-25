#pragma once
namespace Detail
{
	template<typename T, int N>
	struct VectorT
	{
		//static_assert(N <= 4);

		T v[N];

		explicit VectorT() {}
		explicit VectorT(T x) { T r[] = { x,x,x,x }; memcpy(v, r, sizeof(v)); }
		explicit VectorT(T x, T y) { T r[] = { x,y,0,0 }; memcpy(v, r, sizeof(v)); }
		explicit VectorT(T x, T y, T z) { T r[] = { x,y,z,0 }; memcpy(v, r, sizeof(v)); }
		explicit VectorT(T x, T y, T z, T w) { T r[] = { x,y,z,w }; memcpy(v, r, sizeof(v)); }

		template<int NN>
		explicit VectorT(const VectorT<T, NN> a) { for (int i = 0; i < N; i++) Set(i, a.Get(i)); }

		VectorT<T, N> operator +(const VectorT<T, N> a) const { return Op([&](int i) { return v[i] + a.v[i]; }); }
		VectorT<T, N> operator -(const VectorT<T, N> a) const { return Op([&](int i) { return v[i] - a.v[i]; }); }
		VectorT<T, N> operator *(const VectorT<T, N> a) const { return Op([&](int i) { return v[i] * a.v[i]; }); }
		VectorT<T, N> operator *(const T a) const { return Op([&](int i) { return v[i] * a; }); }
		VectorT<T, N> operator /(const VectorT<T, N> a) const { return Op([&](int i) { return v[i] / a.v[i]; }); }
		VectorT<T, N> operator /(const T a) const { return Op([&](int i) { return v[i] / a; }); }

		void operator +=(const VectorT<T, N> a) { *this = *this + a; }
		void operator -=(const VectorT<T, N> a) { *this = *this - a; }
		void operator *=(const VectorT<T, N> a) { *this = *this * a; }
		void operator *=(const T a) { *this = *this * a; }
		void operator /=(const VectorT<T, N> a) { *this = *this / a; }
		void operator /=(const T a) { *this = *this / a; }

		T Dot(const VectorT<T, N> a) const { T r = 0; for (int i = 0; i < N; i++) r += v[i] * a.v[i]; return r; }
		VectorT<T, N> Floor() const { VectorT<T, N> r; for (int i = 0; i < N; i++) r.Set(i, floorf(v[i])); return r; }
		VectorT<T, N> Ceil() const { VectorT<T, N> r; for (int i = 0; i < N; i++) r.Set(i, ceilf(v[i])); return r; }

		friend VectorT<T, N> Max(VectorT<T, N> a, VectorT<T, N> b) { VectorT<T, N> r; for (int i = 0; i < N; i++) r.Set(i, std::max(a.v[i], b.v[i])); return r; }
		friend VectorT<T, N> Min(VectorT<T, N> a, VectorT<T, N> b) { VectorT<T, N> r; for (int i = 0; i < N; i++) r.Set(i, std::min(a.v[i], b.v[i])); return r; }

		T& x() { return v[0]; }
		const T x() const { return v[0]; }
		T& y() { return N > 1 ? v[1] : v[0]; }
		const T y() const { return N > 1 ? v[1] : 0; }
		T& z() { return N > 2 ? v[2] : v[0]; }
		const T z() const { return N > 2 ? v[2] : 0; }
		T& w() { return N > 3 ? v[3] : v[0]; }
		const T w() const { return N > 3 ? v[3] : 0; }

		T& operator[](int i) { static_assert(i < N); return v[i]; }
		const T& operator[](int i) const { static_assert(i < N); return v[i]; }

	protected:
		template<typename F>
		static VectorT<T, N> Op(F&& f) { return VectorT<T, N>(f(0), N > 1 ? f(1) : 0, N > 2 ? f(2) : 0, N > 3 ? f(3) : 0); }

	public:
		T Get(unsigned int i) const { return i < N ? v[i] : 0; }
		void Set(unsigned int i, T a) { if (i < N) v[i] = a; }
	};
}

using Vector2 = Detail::VectorT<float, 2>;
using Vector3 = Detail::VectorT<float, 3>;
using Vector4 = Detail::VectorT<float, 4>;
