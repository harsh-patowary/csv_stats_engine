#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include "Vector.h"
#include "csvstats/csv_reader.h"
#include "Matrix.h"


// using namespace csv;

class Timer {
private:
    std::chrono::high_resolution_clock::time_point m_start;
    const char* m_name;
    std::size_t m_workload;
public:
    Timer(const char* name, std::size_t workload = 0) 
        : m_start(std::chrono::high_resolution_clock::now()), m_name(name), m_workload(workload) {}
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
        std::cout << "[BENCHMARK] " << std::left << std::setw(45) << m_name 
                  << " | Time: " << std::right << std::setw(6) << duration << " ms";
        if (m_workload > 0 && duration > 0) {
            double throughput = (static_cast<double>(m_workload) / duration) / 1000.0; // Million ops/sec
            std::cout << " | Throughput: " << std::fixed << std::setprecision(2) << throughput << " M ops/sec";
        }
        std::cout << "\n";
    }
};

void create_test_csv(const std::string& filename) {
    std::ofstream file(filename);
    file << "Age,Name,Score,Weight\n"  // Col 0: numeric, Col 1: text, Col 2: numeric, Col 3: numeric
         << "25,Alice,95.5,68.2\n"
         << "30,Bob,82.0,75.4\n"
         << "22,Charlie,88.5,62.1\n";
}

int main() {
    std::cout << "=== Comprehensive Production Benchmark: Custom vs STL ===\n\n";

    // -------------------------------------------------------------------------
    // 1. DYNAMIC GROWTH & ALLOCATION (10 Million Elements)
    // -------------------------------------------------------------------------
    std::cout << "--- 1. Dynamic Insertion & Growth (10,000,000 elements) ---\n";
    {
        Timer t("STL std::vector push_back (No Reserve)", 10'000'000);
        std::vector<double> v;
        for (int i = 0; i < 10'000'000; ++i) v.push_back(i);
    }
    {
        Timer t("Custom Vector push_back (No Reserve)", 10'000'000);
        Vector<double> v;
        for (int i = 0; i < 10'000'000; ++i) v.push_back(i);
    }

    std::cout << "\n";

    // -------------------------------------------------------------------------
    // 2. RAM BANDWIDTH TRAVERSAL (50 Million Elements)
    // -------------------------------------------------------------------------
    std::cout << "--- 2. Cache & RAM Bandwidth (50,000,000 elements) ---\n";
    std::vector<double> stl_big(50'000'000, 1.5);
    Vector<double> custom_big(50'000'000);
    custom_big.fill(1.5);

    {
        Timer t("STL std::vector linear read/modify scan", 50'000'000);
        double sum = 0;
        for (std::size_t i = 0; i < 50'000'000; ++i) sum += stl_big[i] * 2.0;
        volatile double g = sum; (void)g;
    }
    {
        Timer t("Custom Vector linear read/modify scan", 50'000'000);
        double sum = 0;
        for (std::size_t i = 0; i < 50'000'000; ++i) sum += custom_big[i] * 2.0;
        volatile double g = sum; (void)g;
    }

    std::cout << "\n";

    // -------------------------------------------------------------------------
    // 3. MATRIX ADDITION COMPARISON (1000 x 1000 = 1,000,000 elements)
    // -------------------------------------------------------------------------
    constexpr std::size_t MAT_DIM = 1000;
    std::cout << "--- 3. Matrix Addition (" << MAT_DIM << "x" << MAT_DIM << ") ---\n";

    std::vector<double> stl_mat_a(MAT_DIM * MAT_DIM, 2.0);
    std::vector<double> stl_mat_b(MAT_DIM * MAT_DIM, 3.0);
    std::vector<double> stl_mat_res(MAT_DIM * MAT_DIM, 0.0);

    Matrix<double> cust_mat_a(MAT_DIM, MAT_DIM); cust_mat_a.fill(2.0);
    Matrix<double> cust_mat_b(MAT_DIM, MAT_DIM); cust_mat_b.fill(3.0);

    {
        Timer t("STL flat std::vector matrix addition", MAT_DIM * MAT_DIM);
        for (std::size_t i = 0; i < MAT_DIM * MAT_DIM; ++i) {
            stl_mat_res[i] = stl_mat_a[i] + stl_mat_b[i];
        }
    }
    {
        Timer t("Custom Matrix<double> addition (operator+)", MAT_DIM * MAT_DIM);
        Matrix<double> cust_res = cust_mat_a + cust_mat_b;
        (void)cust_res;
    }

    std::cout << "\n";

    // -------------------------------------------------------------------------
    // 4. MATRIX MULTIPLICATION COMPARISON O(N^3) (400 x 400)
    // -------------------------------------------------------------------------
    constexpr std::size_t MULT_DIM = 400;
    std::cout << "--- 4. Matrix Multiplication O(N^3) (" << MULT_DIM << "x" << MULT_DIM << ") ---\n";

    // STL flat vector matrix multiplication simulation
    std::vector<double> stl_m1(MULT_DIM * MULT_DIM, 1.01);
    std::vector<double> stl_m2(MULT_DIM * MULT_DIM, 0.99);
    std::vector<double> stl_m_prod(MULT_DIM * MULT_DIM, 0.0);

    Matrix<double> cm1(MULT_DIM, MULT_DIM); cm1.fill(1.01);
    Matrix<double> cm2(MULT_DIM, MULT_DIM); cm2.fill(0.99);

    {
        Timer t("STL flat std::vector matrix multiplication", MULT_DIM * MULT_DIM * MULT_DIM);
        for (std::size_t i = 0; i < MULT_DIM; ++i) {
            for (std::size_t j = 0; j < MULT_DIM; ++j) {
                double sum = 0;
                for (std::size_t k = 0; k < MULT_DIM; ++k) {
                    sum += stl_m1[i * MULT_DIM + k] * stl_m2[k * MULT_DIM + j];
                }
                stl_m_prod[i * MULT_DIM + j] = sum;
            }
        }
    }
    {
        Timer t("Custom Matrix<double> multiplication (operator*)", MULT_DIM * MULT_DIM * MULT_DIM);
        Matrix<double> c_prod = cm1 * cm2;
        (void)c_prod;
    }

    std::cout << "\n=== Comprehensive Benchmark Complete ===\n";

    std::cout << "=== Testing CSV Numeric Column Detection & Parsing ===\n\n";

    std::string test_filename = "test_dataset.csv";
    create_test_csv(test_filename);

    try {
        // 1. Read the CSV file using your CSVReader
        csv::CSVReader reader(test_filename);

        std::cout << "Successfully read file: " << test_filename << "\n";
        std::cout << "Header columns found: " << reader.header().size() << "\n";
        std::cout << "Data rows found: " << reader.rows().size() << "\n\n";

        // 2. Test your parse_numeric_csv function
        // Note: If parse_numeric_csv is a member function of CSVReader, call it via reader.parse_numeric_csv(reader.rows());
        // If it's a standalone function, adjust accordingly.
        Matrix<double> numeric_data = reader.parse_numeric_csv(reader.rows());

        std::cout << "\n--- Resulting Custom Matrix ---\n";
        std::cout << "Matrix Dimensions: " << numeric_data.rows() << " rows x " << numeric_data.cols() << " cols\n";
        std::cout << "Matrix Contents:\n" << numeric_data << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Test failed with exception: " << e.what() << "\n";
    }

    return 0;
}