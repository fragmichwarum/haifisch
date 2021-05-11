#pragma once

#ifndef MATMUL_HPP
#define MATMUL_HPP

#define use_inline
#ifdef use_inline
# define MATRIX_INLINE [[gnu::always_inline]]
#else
#define MATRIX_INLINE
# endif // use_inline
#undef use_inline

#include <iostream>
#include <cassert>
#include <cstring>
#include <memory>

#include <boost/pool/pool_alloc.hpp>


namespace haifisch
{
template <typename T, typename AllocatorTraits>
class matrix_allocator
{
public:
    using value_type = T;
    using allocator_type = typename AllocatorTraits::allocator_type;

    T* allocate(size_t n)
    {
        return allocator.allocate(n);
    }

    static void deallocate(allocator_type& alloc, value_type* ptr, size_t n)
    {
        AllocatorTraits::deallocate(alloc, ptr, n);
    }

    void deallocate(value_type* ptr, size_t n)
    {
        AllocatorTraits::deallocate(allocator, ptr, n);
    }

    static void destroy(allocator_type& alloc, T* ptr)
    {
        alloc.destroy(ptr);
    }


private:
    allocator_type allocator;
};

#include <boost/pool/pool_alloc.hpp>

template  <typename T>
using matrix_allocator_t = matrix_allocator<T, std::allocator_traits<boost::pool_allocator<T>>>;

template <typename T, typename Allocator = matrix_allocator_t<T>>
class vector
{
public:
    MATRIX_INLINE explicit vector(std::size_t size) noexcept
        : allocator()
        , vec(allocator.allocate(size))
        , len(size)
    { }
    MATRIX_INLINE virtual ~vector()
    {
        if (vec)
        {
            delete[] vec;
            vec = nullptr;
        }
    }
    MATRIX_INLINE T& at(std::size_t index) const noexcept
    {
        assert(index < len);
        return vec[index];
    }
    MATRIX_INLINE T& operator[](std::size_t index) const noexcept
    {
        return at(index);
    }
    MATRIX_INLINE constexpr std::size_t size() const noexcept
    {
        return len;
    }
private:
    Allocator allocator;
    T* vec;
    std::size_t len;
};

template <typename T, typename Allocator = matrix_allocator_t<T>>
class matrix;

template <typename T, typename Allocator = matrix_allocator_t<T>>
struct naive_mul_impl
{
    matrix<T> process(const matrix<T>& lhs, const matrix<T>& rhs);
};

template <typename T, typename Allocator = matrix_allocator_t<T>>
struct strassen_mul_impl
{
    matrix<T> process(T* allocated_block, const matrix<T>& lhs, const matrix<T>& rhs);
};

template <typename T>
MATRIX_INLINE matrix<T> transpose(const matrix<T>& rhs) noexcept;

std::uint64_t nearest_power_of_2(std::uint64_t value)
{
    value--;
    value |= value >>  1;
    value |= value >>  2;
    value |= value >>  4;
    value |= value >>  8;
    value |= value >> 16;
    value++;

    return value;
}

template <typename T, typename Allocator>
class matrix
{
public:
    template <typename M_T, typename M_Allocator>
    friend struct strassen_mul_impl;

    MATRIX_INLINE constexpr explicit matrix(std::size_t cols_, std::size_t rows_) noexcept
        : cols(cols_)
        , rows(rows_)
        , mat(allocator.allocate(cols * rows))
    { }
    MATRIX_INLINE matrix(const matrix& rhs) noexcept
    {
        construct(rhs);
    }
    MATRIX_INLINE matrix(matrix&& rhs) noexcept
    {
        mat = rhs.mat;
        cols = rhs.cols;
        rows = rhs.rows;
        rhs.mat = nullptr;
        rhs.rows = 0;
        rhs.cols = 0;
    }
    MATRIX_INLINE virtual ~matrix()
    {
        destroy();
    }
    MATRIX_INLINE void fill(T value) noexcept
    {
        for (std::size_t i = 0; i < cols * rows; i++)
        {
            mat[i] = value;
        }
    }
    MATRIX_INLINE constexpr std::size_t width() const noexcept
    {
        return cols;
    }
    MATRIX_INLINE constexpr std::size_t height() const noexcept
    {
        return rows;
    }
    MATRIX_INLINE T* data() const noexcept
    {
      return mat;
    }
    MATRIX_INLINE constexpr T* at_pointer(std::size_t x, std::size_t y) const noexcept
    {
        return mat + (cols * y + x);
    }
    MATRIX_INLINE constexpr T& at(std::size_t x, std::size_t y) const
    {
        assert(y <= cols);
        assert(x <= rows);

        return mat[cols * y + x];
    }
    MATRIX_INLINE T& operator () (std::size_t x, std::size_t y) noexcept
    {
      return at(x, y);
    }
    MATRIX_INLINE const T& operator () (std::size_t x, std::size_t y) const noexcept
    {
      return at(x, y);
    }
    MATRIX_INLINE matrix& operator = (const matrix& rhs) noexcept
    {
        construct(rhs);
        return *this;
    }
    MATRIX_INLINE matrix& operator = (matrix&& rhs) noexcept
    {
        destroy();
        mat = rhs.mat;
        cols = rhs.cols;
        rows = rhs.rows;
        rhs.mat = nullptr;
        rhs.rows = 0;
        rhs.cols = 0;
        return *this;
    }
    MATRIX_INLINE matrix& operator += (const matrix& rhs) noexcept
    {
        assert(cols == rhs.cols);
        assert(rows == rhs.rows);

        for (std::size_t i = 0; i < cols * rows; i++)
        {
            mat[i] += rhs.mat[i];
        }

        return *this;
    }
    MATRIX_INLINE matrix& operator -= (const matrix& rhs) noexcept
    {
        assert(cols == rhs.cols);
        assert(rows == rhs.rows);

        for (std::size_t i = 0; i < cols * rows; i++)
        {
            mat[i] -= rhs.mat[i];
        }

        return *this;
    }
    MATRIX_INLINE matrix& operator *= (const matrix& rhs) noexcept
    {
        assert(cols == rhs.rows);

        /// If not power of 2.
        if ((cols & (cols - 1)) == 0)
        {
            strassen_mul_impl<T> impl;
            *this = impl.process(*this, rhs);

            rows = rhs.rows;
        }
        else {
            naive_mul_impl<T> impl;
            *this = impl.process(*this, rhs);

            rows = rhs.rows;
        }

        return *this;
    }
    MATRIX_INLINE matrix& operator *= (const T& val) noexcept
    {
        for (std::size_t i = 0; i < cols * rows; i++)
        {
            mat[i] *= val;
        }

        return *this;
    }
    MATRIX_INLINE matrix operator + (const matrix& rhs) const noexcept
    {
        matrix<T> result = *this;
        return result += rhs;
    }
    MATRIX_INLINE matrix operator - (const matrix& rhs) const noexcept
    {
        matrix<T> result = *this;
        return result -= rhs;
    }
    MATRIX_INLINE matrix operator * (const matrix& rhs) const noexcept
    {
        matrix<T> result = *this;
        return result *= rhs;
    }
    MATRIX_INLINE bool operator == (const matrix& other) const noexcept
    {
        if (rows != other.rows) return false;
        if (cols != other.cols) return false;
        return memcmp(mat, other.mat, sizeof(T) * cols * rows) == 0;
    }
    MATRIX_INLINE bool operator != (const matrix& other) const noexcept
    {
        return !(*this == other);
    }

protected:
    void destroy()
    {
        if (mat)
        {
            using alloc_traits = std::allocator_traits<decltype(allocator)>;
            alloc_traits::destroy(allocator, mat);
            alloc_traits::deallocate(allocator, mat, 1);
        }
    }

    MATRIX_INLINE void construct(const matrix& rhs)
    {
        if (mat != nullptr && (cols * rows) == (rhs.cols * rhs.rows))
        {
            std::memset(mat, 0, rows * cols);
        }
        else
        {
            destroy();
            mat = allocator.allocate(rhs.cols * rhs.rows);
        }

        cols = rhs.cols;
        rows = rhs.rows;

        for (std::size_t i = 0; i < cols * rows; i++)
        {
            mat[i] = rhs.mat[i];
        }
    }

private:
    Allocator allocator;
    std::size_t cols;
    std::size_t rows;
    T* mat = nullptr;
};

template <typename T, typename Allocator>
matrix<T> naive_mul_impl<T, Allocator>::process(const matrix<T>& lhs, const matrix<T>& rhs)
{
    matrix<T> transposed = transpose(lhs);
    matrix<T> result(lhs.height(), lhs.width());

    #pragma omp parallel
    {
        #pragma omp for nowait collapse(2)
        for (std::size_t i = 0; i < lhs.height(); i++)
        {
            for (std::size_t j = 0; j < rhs.height(); j++)
            {
                T accumulator = {};
                for (std::size_t k = 0; k < rhs.width(); k++)
                {
                    accumulator += (transposed(k, i) * rhs(k, j));
                }
                result(i, j) = accumulator;
            }
        }
    }

    return result;
}

template <typename T, typename Allocator>
matrix<T> strassen_mul_impl<T, Allocator>::process(T* allocated_block, const matrix<T>& lhs, const matrix<T>& rhs)
{
    (void) allocated_block;

    if (lhs.height() == 1)
    {
        matrix<T> result(1, 1);
        result(0, 0) = lhs(0, 0) * rhs(0, 0);
        return result;
    }

    matrix<T> C(rhs.height(), rhs.height());
    size_t k = lhs.height() / 2;

    matrix<T> a11(k, k);
    matrix<T> a12(k, k);
    matrix<T> a21(k, k);
    matrix<T> a22(k, k);

    matrix<T> b11(k, k);
    matrix<T> b12(k, k);
    matrix<T> b21(k, k);
    matrix<T> b22(k, k);

    for (size_t i = 0; i < k; ++i)
    {
        for (size_t j = 0; j < k; ++j)
        {
            a11(i, j) = lhs(i, j);
            a12(i, j) = lhs(i, k + j);
            a21(i, j) = lhs(k + i, j);
            a22(i, j) = lhs(k + i, k + j);

            b11(i, j) = rhs(i, j);
            b12(i, j) = rhs(i, k + j);
            b21(i, j) = rhs(k + i, j);
            b22(i, j) = rhs(k + i, k + j);
        }
    }

    matrix<T> p1 = process(a11, b12 - b22);
    matrix<T> p2 = process(a11 + a12, b22);
    matrix<T> p3 = process(a21 + a22, b11);
    matrix<T> p4 = process(a22, b21 - b11);
    matrix<T> p5 = process(a11 + a22, b11 + b22);
    matrix<T> p6 = process(a12 - a22, b21 + b22);
    matrix<T> p7 = process(a11 - a21, b11 + b12);

    matrix<T> c11 = ((p5 + p4) + p6) - p2;
    matrix<T> c12 = p1 + p2;
    matrix<T> c21 = p3 + p4;
    matrix<T> c22 = ((p5 + p1) - p3) - p7;

    for (size_t i = 0; i < k; ++i)
    {
        for (size_t j = 0; j < k; ++j)
        {
            C(i, j) = c11(i, j);
            C(i, j + k) = c12(i, j);
            C(k + i, j) = c21(i, j);
            C(k + i, k + j) = c22(i, j);
        }
    }

    return C;
}

template <typename T>
MATRIX_INLINE std::ostream& operator << (std::ostream& ostream, const vector<T>& vec)
{
    ostream << "[ ";
    for (std::size_t i = 0; i < vec.size(); i++)
    {
        ostream << vec[i] << ' ';
    }
    ostream << ']';
    return ostream;
}

template <typename T>
std::ostream& operator << (std::ostream& ostream, const matrix<T>& mat)
{
    for (std::size_t cols = 0; cols < mat.width(); ++cols)
    {
        for (std::size_t rows = 0; rows < mat.height(); ++rows)
        {
            ostream << mat(cols, rows) << " ";
        }
        ostream << std::endl;
    }
    return ostream;
}

template <typename T>
MATRIX_INLINE vector<T> operator *= (matrix<T>& mat, const vector<T>& vec) noexcept
{
    assert(mat.height() == vec.size());

    vector<T> result(vec.size());
    for (std::size_t i = 0; i < mat.width(); i++)
    {
        T accumulator = {};
        for (std::size_t j = 0; j < mat.height(); j++)
        {
            accumulator += mat.at(i, j) * vec[i];
        }
        result.at(i) = accumulator;
    }
    return result;
}

template <typename T>
MATRIX_INLINE matrix<T> transpose(const matrix<T>& rhs) noexcept
{
    std::size_t height = rhs.height();
    std::size_t width = rhs.width();
    matrix<T> transposed(height, width);

    for (std::size_t i = 0; i < height; ++i)
    {
        for (std::size_t j = 0; j < height; ++j)
        {
            transposed(i, j) = rhs(j, i);
        }
    }

    return transposed;
}
} // namespace haifisch

#undef MATRIX_INLINE
#undef matrix_num_threads
#endif // MATMUL_HPP
