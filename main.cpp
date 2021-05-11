#include <chrono>
#include <gtest/gtest.h>
#include <numeric>
#include <random>

#include <boost/numeric/ublas/matrix.hpp>

#include "haifisch/matrix.hpp"
#include "util/func_benchmark.hpp"
#include "util/logger.hpp"

using namespace haifisch;

template <typename T>
void debug_info(std::string_view label, const matrix<T>& rhs) noexcept
{
    logger(logflag::debug) << label << " capacity:   " << rhs.width() * rhs.height();
    logger(logflag::debug) << label << " memory:     " << ((rhs.width() * rhs.height()) * sizeof (T)) / 1024.0 / 1024.0 << " MiB.";
}

template <typename T>
void test_matrix_equal(const matrix<T>& lhs, const matrix<T>& rhs) noexcept
{
    (lhs == rhs)
        ? logger(logflag::info  | logflag::green) << "equal test: " << "matrix1 == matrix2"
        : logger(logflag::error | logflag::red  ) << "equal test: " << "matrix1 != matrix2";
}

template <typename T>
void test_matrix_not_equal(const matrix<T>& lhs, const matrix<T>& rhs) noexcept
{
    (lhs != rhs)
        ? logger(logflag::info  | logflag::green) << "not equal test: " << "matrix1 != matrix2"
        : logger(logflag::error | logflag::red  ) << "not equal test: " << "matrix1 == matrix2";
}

template <typename T>
void test_matrix_add(const matrix<T>& lhs, const matrix<T>& rhs, const matrix<T>& control) noexcept
{
    matrix<T> res = lhs + rhs;
    ((lhs + rhs) == control)
        ? logger(logflag::info  | logflag::green) << "add test: " << "matrix1 + matrix2 == control"
        : logger(logflag::error | logflag::red  ) << "add test: " << "matrix1 + matrix2 != control";
}

template <typename T>
void test_matrix_sub(const matrix<T>& lhs, const matrix<T>& rhs, const matrix<T>& control) noexcept
{
    ((lhs - rhs) == control)
        ? logger(logflag::info  | logflag::green) << "sub test: " << "matrix1 - matrix2 == control"
        : logger(logflag::error | logflag::red  ) << "sub test: " << "matrix1 - matrix2 != control";
}

template <typename T>
void test_matrix_mul(const matrix<T>& lhs, const matrix<T>& rhs, const matrix<T>& control) noexcept
{
    ((lhs * rhs) == control)
        ? logger(logflag::info  | logflag::green) << "mul test: " << "matrix1 * matrix2 == control"
        : logger(logflag::error | logflag::red  ) << "mul test: " << "matrix1 * matrix2 != control";
}

template <typename T>
void transpose_assert(matrix<T>& rhs, const matrix<T>& control) noexcept
{
    rhs.transpose();

    ((rhs == control))
        ? logger(logflag::info  | logflag::green) << "transpose test"
        : logger(logflag::error | logflag::red  ) << "transpose test";
}

template <typename T>
void test_matrix_transpose(std::size_t mat_x, std::size_t mat_y)
{
    matrix<T> mat(mat_x, mat_y);
    boost::numeric::ublas::matrix<T> boost_mat(mat_x, mat_y);

    for (std::size_t i = 0; i < mat_x; i++)
    {
        for (std::size_t j = 0; j < mat_y; j++)
        {
            mat(i, j) = i;
            boost_mat(i, j) = i;
        }
    }

    boost::numeric::ublas::matrix<T> boost_transpose_mat(mat_y, mat_x);

    debug_info("transpose mat", mat);

    auto start = std::chrono::high_resolution_clock::now();

    mat = transpose(mat);

    auto timeSpent = std::chrono::high_resolution_clock::now() - start;
    std::cout << "daz matrix transpose: " << mat.width() << "x" << mat.height() << "   -> " << "\033[0;31m" << std::chrono::duration_cast<std::chrono::duration<float>>(timeSpent).count() << " s." << "\033[0;0m" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();

    boost_transpose_mat = boost::numeric::ublas::trans(boost_mat);

    auto timeSpent2 = std::chrono::high_resolution_clock::now() - start2;
    std::cout << "boost matrix transpose: " << boost_transpose_mat.size1() << "x" << boost_transpose_mat.size2() << " -> " << "\033[0;31m" << std::chrono::duration_cast<std::chrono::duration<float>>(timeSpent2).count() << " s." << "\033[0;0m"  << std::endl;

    std::vector<T> mat_v;
    std::vector<T> boost_v;

    for (std::size_t i = 0; i < mat_x; i++)
    {
        for (std::size_t j = 0; j < mat_y; j++)
        {
            mat_v.push_back(mat(i, j));
            boost_v.push_back(boost_transpose_mat(i, j));
        }
    }

    if (mat_v == boost_v)
    {
        std::cout << "is same with " << typeid(T).name() << " " << mat_x << "x" << mat_y << " boost matrix\n";
    }
    else {
        std::cout << "ERROR: matrix mismatch with " << typeid(T).name() << " " << mat_x << "x" << mat_y << " boost matrix\n";
        exit(-1);
    }
}

template <typename T>
void test_matrix_construct()
{
    std::size_t matrix_size = 3'000;

    matrix<T> stack_mat(matrix_size, matrix_size);
    stack_mat.fill(10);

    matrix<T> stack_mat2 = std::move(stack_mat);
    matrix<T> stack_mat3 = stack_mat;
}

template <typename T>
void test_arithmetic()
{
    const std::size_t iterations  = 1;
    const std::size_t matrix_size = 1000;
    T           lhs_data    = 10;
    T           rhs_data    = 5;

    matrix<T> lhs(matrix_size, matrix_size);
    matrix<T> rhs(matrix_size, matrix_size);
    lhs.fill(lhs_data);
    rhs.fill(rhs_data);

    debug_info("lhs matrix ", lhs);
    debug_info("rhs matrix ", rhs);

    matrix<T> control_add_mat(matrix_size, matrix_size);
    matrix<T> control_sub_mat(matrix_size, matrix_size);

    control_add_mat.fill(lhs_data + rhs_data);
    control_sub_mat.fill(lhs_data - rhs_data);

    test_func_speed(iterations, test_matrix_equal<T>, lhs, lhs);
    test_func_speed(iterations, test_matrix_add<T>, lhs, rhs, control_add_mat);
    test_func_speed(iterations, test_matrix_sub<T>, lhs, rhs, control_sub_mat);
}

void test_multiplication()
{
    const std::size_t matrix_size = 1000;

    matrix<int> int_mat(matrix_size, matrix_size);
    matrix<float> float_mat(matrix_size, matrix_size);
    matrix<double> double_mat(matrix_size, matrix_size);
    int_mat.fill(10);
    float_mat.fill(10.1);
    double_mat.fill(10.1);

    int_mat *= int_mat;
    float_mat *= float_mat;
    double_mat *= double_mat;

    logger(logflag::debug | logflag::yellow | logflag::spaces) << "int mat    " << int_mat    (0, 0);
    logger(logflag::debug | logflag::yellow | logflag::spaces) << "float mat  " << float_mat  (0, 0);
    logger(logflag::debug | logflag::yellow | logflag::spaces) << "double mat " << double_mat (0, 0);
}


#include <boost/numeric/ublas/matrix.hpp>

template <typename T>
std::ostream& operator<<(std::ostream& ostream, const boost::numeric::ublas::matrix<T>& matrix)
{
    for (std::size_t i = 0; i < matrix.size1(); ++i)
    {
        ostream << '[';
        ostream << ' ';
        for (std::size_t j = 0; j < matrix.size2(); ++j)
        {
            ostream << matrix(i, j) << ' ';
        }
        ostream << ']';
        ostream << '\n';
    }
    return ostream;
}

template <typename T>
void fill_impl( matrix<T>& mat,
                matrix<T>& mat_data,
                boost::numeric::ublas::matrix<T>& boost_mat,
                boost::numeric::ublas::matrix<T>& boost_mat_data)
{
    for (std::size_t i = 0; i < mat.width(); i++)
    {
        for (std::size_t j = 0; j < mat.height(); j++)
        {
            int seed = rand() % 25;
            mat(i, j) = i + seed;
            mat_data(i, j) = j + seed;
            boost_mat(i, j) = i + seed;
            boost_mat_data(i, j) = j + seed;
        }
    }
}

template <typename T>
void boost_ublas_test_mul(std::size_t mat_x, std::size_t mat_y)
{
    matrix<T> mat(mat_x, mat_y);
    matrix<T> mat_data(mat_x, mat_y);
    boost::numeric::ublas::matrix<T> boost_mat(mat_x, mat_y);
    boost::numeric::ublas::matrix<T> boost_mat_data(mat_x, mat_y);

    fill_impl(mat, mat_data, boost_mat, boost_mat_data);

    matrix<T> mult_mat(mat_x, mat_y);
    boost::numeric::ublas::matrix<T> boost_mult_mat(mat_x, mat_y);

    debug_info("mult_mat", mult_mat);

    auto start = std::chrono::high_resolution_clock::now();

    mult_mat = mat * mat_data;

    auto timeSpent = std::chrono::high_resolution_clock::now() - start;
    std::cout << "daz matrix: " << mult_mat.width() << "x" << mult_mat.height() << "   -> " << "\033[0;31m" << std::chrono::duration_cast<std::chrono::duration<float>>(timeSpent).count() << " s." << "\033[0;0m" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();

    boost_mult_mat = boost::numeric::ublas::prod(boost_mat, boost_mat_data);

    auto timeSpent2 = std::chrono::high_resolution_clock::now() - start2;
    std::cout << "boost matrix: " << boost_mult_mat.size1() << "x" << boost_mult_mat.size2() << " -> " << "\033[0;31m" << std::chrono::duration_cast<std::chrono::duration<float>>(timeSpent2).count() << " s." << "\033[0;0m"  << std::endl;

    std::vector<T> mat_v;
    std::vector<T> boost_v;

    for (std::size_t i = 0; i < mat_x; i++)
    {
        for (std::size_t j = 0; j < mat_y; j++)
        {
            if (mult_mat(i, j) != boost_mult_mat(i, j))
            {
                std::cout << "ERROR: matrix mismatch with " << typeid(T).name() << " " << mat_x << "x" << mat_y << " boost matrix\n";
                exit(-1);
            }
        }
    }
}

template <typename T>
void multiply_many(const matrix<T>& m)
{
    matrix<T> res = m * m;
    std::cout << res(0, 0) << std::endl;
}

int main()
{
//    test_matrix_transpose<int>(100, 100);
//    test_matrix_transpose<int>(250, 250);
//    test_matrix_transpose<int>(500, 500);
//    test_matrix_transpose<int>(750, 750);
//    test_matrix_transpose<int>(1000, 1000);
////    test_matrix_transpose<int>(2000, 2000);
////    test_matrix_transpose<int>(3000, 3000);
////    test_matrix_transpose<int>(5000, 5000);
////    test_matrix_transpose<int>(10000, 10000);

    boost_ublas_test_mul<double>(8, 8);
    boost_ublas_test_mul<double>(16, 16);
    boost_ublas_test_mul<double>(32, 32);
    boost_ublas_test_mul<double>(33, 33);
//    boost_ublas_test_mul<double>(64, 64);
    boost_ublas_test_mul<double>(127, 127);
//    boost_ublas_test_mul<double>(128, 128);
    boost_ublas_test_mul<double>(129, 129);
//    boost_ublas_test_mul<double>(256, 256);
//    boost_ublas_test_mul<double>(512, 512);


    boost_ublas_test_mul<double>(10, 10);
    boost_ublas_test_mul<double>(50, 50);
    boost_ublas_test_mul<double>(60, 60);
    boost_ublas_test_mul<double>(70, 70);
    boost_ublas_test_mul<double>(80, 80);
    boost_ublas_test_mul<double>(90, 90);
    boost_ublas_test_mul<double>(100, 100);
    boost_ublas_test_mul<double>(110, 110);
    boost_ublas_test_mul<double>(120, 120);
    boost_ublas_test_mul<double>(130, 130);
    boost_ublas_test_mul<double>(140, 140);
    boost_ublas_test_mul<double>(200, 200);
    boost_ublas_test_mul<double>(255, 255);
//    boost_ublas_test_mul<double>(256, 256);
    boost_ublas_test_mul<double>(257, 257);
    boost_ublas_test_mul<double>(300, 300);
    boost_ublas_test_mul<double>(301, 301);
    boost_ublas_test_mul<double>(400, 400);
    boost_ublas_test_mul<double>(450, 450);
    boost_ublas_test_mul<double>(475, 475);
    boost_ublas_test_mul<double>(490, 490);
    boost_ublas_test_mul<double>(500, 500);
    boost_ublas_test_mul<double>(511, 511);
//    boost_ublas_test_mul<double>(512, 512);
    boost_ublas_test_mul<double>(513, 513);
    boost_ublas_test_mul<double>(600, 600);
    boost_ublas_test_mul<double>(650, 650);
    boost_ublas_test_mul<double>(700, 700);

    boost_ublas_test_mul<double>(1023, 1023);
    boost_ublas_test_mul<double>(1024, 1024);
    boost_ublas_test_mul<double>(1025, 1025);


    boost_ublas_test_mul<double>(4095, 4095);
    boost_ublas_test_mul<double>(4096, 4096);
    boost_ublas_test_mul<double>(4097, 4097);
    boost_ublas_test_mul<double>(8192, 8192);

    boost_ublas_test_mul<char>(2000, 2000);
    boost_ublas_test_mul<short>(2000, 2000);
    boost_ublas_test_mul<int>(2000, 2000);
    boost_ublas_test_mul<long>(2000, 2000);
    boost_ublas_test_mul<unsigned long long>(2000, 2000);
    boost_ublas_test_mul<float>(2000, 2000);
    boost_ublas_test_mul<double>(2000, 2000);
    boost_ublas_test_mul<long double>(2000, 2000);

//    boost_ublas_test_mul<int>(3000, 3000);
//    boost_ublas_test_mul<double>(4000, 4000);
}
